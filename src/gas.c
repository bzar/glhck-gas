#include "gas.h"
#include "internal.h"

#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

gasAnimation* gasNumberAnimationNewFromTo(gasNumberAnimationTarget const target, gasEasingType const easing,
                                          float const from, float const to, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_FROM_TO, from, to, duration);
}

gasAnimation* gasNumberAnimationNewFromDelta(gasNumberAnimationTarget const target, gasEasingType const easing,
                                             float const from, float const delta, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_FROM_DELTA, from, delta, duration);
}

gasAnimation* gasNumberAnimationNewDeltaTo(gasNumberAnimationTarget const target, gasEasingType const easing,
                                           float const delta, float const to, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_DELTA_TO, delta, to, duration);
}

gasAnimation* gasNumberAnimationNewFrom(gasNumberAnimationTarget const target, gasEasingType const easing,
                                        float const from, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_FROM, from, 0.0f, duration);
}

gasAnimation* gasNumberAnimationNewTo(gasNumberAnimationTarget const target, gasEasingType const easing,
                                      float const to, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_TO, 0.0f, to, duration);
}

gasAnimation* gasNumberAnimationNewDelta(gasNumberAnimationTarget const target, gasEasingType const easing,
                                         float const delta, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_DELTA, 0.0f, delta, duration);
}

void gasAnimationFree(gasAnimation* animation)
{
  switch (animation->type)
  {
    case GAS_ANIMATION_TYPE_NUMBER: free(animation); break;
    case GAS_ANIMATION_TYPE_PAUSE: free(animation); break;
    case GAS_ANIMATION_TYPE_SEQUENTIAL:
    {
      int i;
      for(i = 0; i < animation->sequentialAnimation.numChildren; ++i)
      {
        gasAnimationFree(animation->sequentialAnimation.children[i]);
      }
      free(animation->sequentialAnimation.children);
      free(animation);
      break;
    }
    case GAS_ANIMATION_TYPE_PARALLEL:
    {
      int i;
      for(i = 0; i < animation->parallelAnimation.numChildren; ++i)
      {
        gasAnimationFree(animation->parallelAnimation.children[i]);
      }
      free(animation->parallelAnimation.children);
      free(animation);
      break;
    }
    case GAS_ANIMATION_TYPE_MODEL:
    {
      glhckAnimatorFree(animation->modelAnimation.animator);
      free(animation);
      break;
    }
    default: assert(0);
  }
}

gasAnimation* gasSequentialAnimationNew(gasAnimation** children, const unsigned int numChildren)
{
  gasAnimation* animation = _gasAnimationNew(GAS_ANIMATION_TYPE_SEQUENTIAL);
  animation->sequentialAnimation.numChildren = numChildren;
  animation->sequentialAnimation.currentIndex = 0;
  animation->sequentialAnimation.children = calloc(numChildren, sizeof(gasAnimation*));
  memcpy(animation->sequentialAnimation.children, children, numChildren * sizeof(gasAnimation*));

  return animation;
}


gasAnimation* gasPauseAnimationNew(const float duration)
{
  gasAnimation* animation = _gasAnimationNew(GAS_ANIMATION_TYPE_PAUSE);
  animation->pauseAnimation.duration = duration;
  animation->pauseAnimation.time = 0.0f;
  return animation;
}


gasAnimation* gasParallelAnimationNew(gasAnimation** children, const unsigned int numChildren)
{
  gasAnimation* animation = _gasAnimationNew(GAS_ANIMATION_TYPE_PARALLEL);
  animation->parallelAnimation.numChildren = numChildren;
  animation->parallelAnimation.children = calloc(numChildren, sizeof(gasAnimation*));
  memcpy(animation->parallelAnimation.children, children, numChildren * sizeof(gasAnimation*));
  return animation;
}

gasAnimation* gasModelAnimationNew(glhckObject* model, const char* name, float duration)
{
  gasAnimation* animation = _gasAnimationNew(GAS_ANIMATION_TYPE_MODEL);
  glhckAnimator* animator = glhckAnimatorNew();

  unsigned int numAnimations;
  glhckAnimation** animations = glhckObjectAnimations(model, &numAnimations);
  glhckAnimation* modelAnimation = NULL;
  int i;
  for(i = 0; i < numAnimations; ++i)
  {
    if(strcmp(name, glhckAnimationGetName(animations[i])) == 0)
    {
      modelAnimation = animations[i];
      break;
    }
  }

  assert(modelAnimation);
  glhckAnimatorAnimation(animator, modelAnimation);
  animation->modelAnimation.animationDuration = glhckAnimationGetDuration(modelAnimation);

  unsigned int numBones;
  glhckBone** bones = glhckObjectBones(model, &numBones);
  glhckAnimatorInsertBones(animator, bones, numBones);

  animation->modelAnimation.animator = animator;
  animation->modelAnimation.duration = duration;

  return animation;
}

gasBoolean gasAnimate(gasAnimation* animation, glhckObject* object, float const delta)
{
  _gasAnimate(animation, object, delta);
  return animation->state != GAS_ANIMATION_STATE_FINISHED ? GAS_TRUE : GAS_FALSE;
}

void gasAnimationLoopTimes(gasAnimation* animation, unsigned int times)
{
  animation->loops = times;
}


void gasAnimationLoop(gasAnimation* animation)
{
  animation->loops = -1;
}


void gasAnimationReset(gasAnimation* animation)
{
  animation->loop = 0;
  _gasAnimationResetCurrentLoop(animation);
}

// INTERNAL

gasAnimation* _gasAnimationNew(_gasAnimationType type)
{
  gasAnimation* animation = calloc(1, sizeof(_gasAnimation));
  animation->state = GAS_ANIMATION_STATE_NOT_STARTED;
  animation->type = type;
  animation->loops = 1;
  animation->loop = 0;
  return animation;
}

gasAnimation* _gasNumberAnimationNew(gasNumberAnimationTarget const target, gasEasingType const easing,
                                     _gasNumberAnimationType const type, float const a, float const b, float const duration)
{
  gasAnimation* animation = _gasAnimationNew(GAS_ANIMATION_TYPE_NUMBER);
  animation->numberAnimation.type = type;
  animation->numberAnimation.target = target;
  animation->numberAnimation.a = a;
  animation->numberAnimation.b = b;
  animation->numberAnimation.duration = duration;
  animation->numberAnimation.easing = easing;
  animation->numberAnimation.time = 0.0f;
  return animation;
}

float _gasAnimate(gasAnimation* animation, glhckObject* object, float const delta)
{
  if (animation->state == GAS_ANIMATION_STATE_FINISHED)
    return delta;

  float left = delta;

  while (_gasLoopsLeft(animation) && left > 0)
  {
    switch (animation->type)
    {
      case GAS_ANIMATION_TYPE_NUMBER: left = _gasAnimateNumberAnimation(animation, object, delta); break;
      case GAS_ANIMATION_TYPE_PAUSE: left = _gasAnimatePauseAnimation(animation, object, delta); break;
      case GAS_ANIMATION_TYPE_SEQUENTIAL: left = _gasAnimateSequentialAnimation(animation, object, delta); break;
      case GAS_ANIMATION_TYPE_PARALLEL: left = _gasAnimateParallelAnimation(animation, object, delta); break;
      case GAS_ANIMATION_TYPE_MODEL: left = _gasAnimateModelAnimation(animation, object, delta); break;
      default: assert(0);
    }

    if (animation->state == GAS_ANIMATION_STATE_FINISHED)
    {
      animation->loop += 1;
      if(_gasLoopsLeft(animation))
      {
        _gasAnimationResetCurrentLoop(animation);
      }

    }
  }

  return left;
}

float _gasAnimateNumberAnimation(gasAnimation* animation, glhckObject* object, float const delta)
{
  if (animation->state == GAS_ANIMATION_STATE_NOT_STARTED)
  {
    switch (animation->numberAnimation.type)
    {
      case GAS_NUMBER_ANIMATION_TYPE_FROM:
      {
        animation->numberAnimation.b = _gasNumberAnimationGetTargetValue(animation->numberAnimation.target, object);
        break;
      }
      case GAS_NUMBER_ANIMATION_TYPE_TO:
      {
        animation->numberAnimation.a = _gasNumberAnimationGetTargetValue(animation->numberAnimation.target, object);
        break;
      }
      case GAS_NUMBER_ANIMATION_TYPE_DELTA:
      {
        animation->numberAnimation.a = _gasNumberAnimationGetTargetValue(animation->numberAnimation.target, object);
        break;
      }
      default: break;
    }
  }

  animation->numberAnimation.time += delta;

  float const relativeTime = animation->numberAnimation.time / animation->numberAnimation.duration;

  animation->state = animation->numberAnimation.time >= animation->numberAnimation.duration
      ? GAS_ANIMATION_STATE_FINISHED
      : GAS_ANIMATION_STATE_RUNNING;

  float progress;
  float t = _gasClamp(relativeTime, 0, 1);

  switch (animation->numberAnimation.easing)
  {
    case GAS_EASING_LINEAR: progress = t; break;
    case GAS_EASING_QUAD_IN: progress = t * t; break;
    case GAS_EASING_QUAD_OUT: progress = 2 * t - t * t; break;
    default: assert(0);
  }

  float value;
  switch (animation->numberAnimation.type)
  {
    case GAS_NUMBER_ANIMATION_TYPE_FROM_TO:
    case GAS_NUMBER_ANIMATION_TYPE_FROM:
    case GAS_NUMBER_ANIMATION_TYPE_TO:
    {
      value = animation->numberAnimation.a + (animation->numberAnimation.b - animation->numberAnimation.a) * progress;
      break;
    }
    case GAS_NUMBER_ANIMATION_TYPE_FROM_DELTA:
    case GAS_NUMBER_ANIMATION_TYPE_DELTA:
    {
      value = animation->numberAnimation.a + animation->numberAnimation.b * progress;
      break;
    }
    case GAS_NUMBER_ANIMATION_TYPE_DELTA_TO:
    {
      value = (animation->numberAnimation.b - animation->numberAnimation.a) + animation->numberAnimation.a * progress;
      break;
    }
    default: assert(0);
  }

  _gasNumberAnimationSetTargetValue(animation->numberAnimation.target, object, value);

  return animation->numberAnimation.time >= animation->numberAnimation.duration
      ? animation->numberAnimation.time - animation->numberAnimation.duration
      : 0;
}

float _gasAnimatePauseAnimation(gasAnimation* animation, glhckObject* object, float const delta)
{
  animation->pauseAnimation.time += delta;
  animation->state = animation->pauseAnimation.time >= animation->pauseAnimation.duration
      ? GAS_ANIMATION_STATE_FINISHED
      : GAS_ANIMATION_STATE_RUNNING;
  return animation->pauseAnimation.time >= animation->pauseAnimation.duration
      ? animation->pauseAnimation.time - animation->pauseAnimation.duration
      : 0;
}

float _gasAnimateSequentialAnimation(gasAnimation* animation, glhckObject* object, float const delta)
{
  float left = delta;
  while (left > 0 && animation->sequentialAnimation.currentIndex < animation->sequentialAnimation.numChildren)
  {
    gasAnimation* child = animation->sequentialAnimation.children[animation->sequentialAnimation.currentIndex];
    left = _gasAnimate(child, object, left);

    if(left > 0)
    {
      animation->sequentialAnimation.currentIndex += 1;
    }
  }

  animation->state = animation->sequentialAnimation.currentIndex >= animation->sequentialAnimation.numChildren
      ? GAS_ANIMATION_STATE_FINISHED
      : GAS_ANIMATION_STATE_RUNNING;

  return left;
}

float _gasAnimateParallelAnimation(gasAnimation* animation, glhckObject* object, float const delta)
{
  float minLeft = delta;

  int i;
  for (i = 0; i < animation->parallelAnimation.numChildren; ++i)
  {
    gasAnimation* child = animation->parallelAnimation.children[i];
    float left = _gasAnimate(child, object, delta);
    minLeft = left < minLeft ? left : minLeft;
  }

  animation->state = minLeft > 0
      ? GAS_ANIMATION_STATE_FINISHED
      : GAS_ANIMATION_STATE_RUNNING;

  return minLeft;
}

float _gasAnimateModelAnimation(gasAnimation* animation, glhckObject* object, float const delta)
{
  if(animation->modelAnimation.duration <= 0.0f) {
    animation->state = GAS_ANIMATION_STATE_FINISHED;
    return delta;
  }
  animation->modelAnimation.time += delta;
  float position = animation->modelAnimation.time / animation->modelAnimation.duration;
  glhckAnimatorUpdate(animation->modelAnimation.animator, _gasClamp(position, 0.0f, 1.0f) * animation->modelAnimation.animationDuration / 25);
  glhckAnimatorTransform(animation->modelAnimation.animator, object);
  if(animation->modelAnimation.time > animation->modelAnimation.duration)
  {
    animation->state = GAS_ANIMATION_STATE_FINISHED;
    return animation->modelAnimation.time - animation->modelAnimation.duration;
  }
  else
  {
    animation->state = GAS_ANIMATION_STATE_RUNNING;
    return 0.0f;
  }
}

void _gasAnimationResetCurrentLoop(gasAnimation* animation)
{
  animation->state = GAS_ANIMATION_STATE_NOT_STARTED;

  switch (animation->type)
  {
    case GAS_ANIMATION_TYPE_NUMBER: return _gasAnimationResetNumberAnimation(animation); break;
    case GAS_ANIMATION_TYPE_PAUSE: return _gasAnimationResetPauseAnimation(animation); break;
    case GAS_ANIMATION_TYPE_SEQUENTIAL: return _gasAnimationResetSequentialAnimation(animation); break;
    case GAS_ANIMATION_TYPE_PARALLEL: return _gasAnimationResetParallelAnimation(animation); break;
    case GAS_ANIMATION_TYPE_MODEL: return _gasAnimationResetModelAnimation(animation); break;
    default: assert(0);
  }
}

void _gasAnimationResetNumberAnimation(gasAnimation* animation)
{
  animation->numberAnimation.time = 0.0f;
}

void _gasAnimationResetPauseAnimation(gasAnimation* animation)
{
  animation->pauseAnimation.time = 0.0f;
}

void _gasAnimationResetSequentialAnimation(gasAnimation* animation)
{
  animation->sequentialAnimation.currentIndex = 0;
  int i;
  for (i = 0; i < animation->sequentialAnimation.numChildren; ++i)
  {
    gasAnimation* child = animation->sequentialAnimation.children[i];
    gasAnimationReset(child);
  }
}

void _gasAnimationResetParallelAnimation(gasAnimation* animation)
{
  int i;
  for (i = 0; i < animation->parallelAnimation.numChildren; ++i)
  {
    gasAnimation* child = animation->parallelAnimation.children[i];
    gasAnimationReset(child);
  }
}

void _gasAnimationResetModelAnimation(gasAnimation* animation)
{
  animation->modelAnimation.time = 0.0f;
}

float _gasNumberAnimationGetTargetValue(gasNumberAnimationTarget target, glhckObject* object)
{
  switch (target)
  {
    case GAS_NUMBER_ANIMATION_TARGET_X: return glhckObjectGetPosition(object)->x;
    case GAS_NUMBER_ANIMATION_TARGET_Y: return glhckObjectGetPosition(object)->y;
    case GAS_NUMBER_ANIMATION_TARGET_Z: return glhckObjectGetPosition(object)->z;
    case GAS_NUMBER_ANIMATION_TARGET_ROT_X: return glhckObjectGetRotation(object)->x;
    case GAS_NUMBER_ANIMATION_TARGET_ROT_Y: return glhckObjectGetRotation(object)->y;
    case GAS_NUMBER_ANIMATION_TARGET_ROT_Z: return glhckObjectGetRotation(object)->z;
    default: assert(0);
  }
}

void _gasNumberAnimationSetTargetValue(gasNumberAnimationTarget target, glhckObject* object, float const value)
{
  switch (target)
  {
    case GAS_NUMBER_ANIMATION_TARGET_X:
    case GAS_NUMBER_ANIMATION_TARGET_Y:
    case GAS_NUMBER_ANIMATION_TARGET_Z:
    {
      kmVec3 position = *glhckObjectGetPosition(object);
      switch (target)
      {
        case GAS_NUMBER_ANIMATION_TARGET_X: position.x = value; break;
        case GAS_NUMBER_ANIMATION_TARGET_Y: position.y = value; break;
        case GAS_NUMBER_ANIMATION_TARGET_Z: position.z = value; break;
        default: assert(0);
      }
      glhckObjectPosition(object, &position);
      break;
    }
    case GAS_NUMBER_ANIMATION_TARGET_ROT_X:
    case GAS_NUMBER_ANIMATION_TARGET_ROT_Y:
    case GAS_NUMBER_ANIMATION_TARGET_ROT_Z:
    {
      kmVec3 rotation = *glhckObjectGetRotation(object);
      switch (target)
      {
        case GAS_NUMBER_ANIMATION_TARGET_ROT_X: rotation.x = value; break;
        case GAS_NUMBER_ANIMATION_TARGET_ROT_Y: rotation.y = value; break;
        case GAS_NUMBER_ANIMATION_TARGET_ROT_Z: rotation.z = value; break;
        default: assert(0);
      }
      glhckObjectRotation(object, &rotation);
      break;
    }
    default: assert(0);
  }
}

float _gasClamp(float const value, float const minValue, float const maxValue)
{
  return value <= minValue ? minValue : value >= maxValue ? maxValue : value;
}

float _gasLoopsLeft(_gasAnimation* animation)
{
  return animation->loops > animation->loop || animation->loops == -1;
}

gasAnimationState gasAnimationGetState(gasAnimation* animation)
{
  return animation->state;
}


gasAnimation* gasAnimationClone(gasAnimation* animation)
{
  gasAnimation* newAnimation = _gasAnimationNew(animation->type);
  *newAnimation = *animation;
  return newAnimation;
}
