#include "gas.h"
#include "internal.h"

#include <assert.h>
#include <stdlib.h>
#include <memory.h>

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

gasAnimation* _gasNumberAnimationNew(gasNumberAnimationTarget const target, gasEasingType const easing,
                                     _gasNumberAnimationType const type, float const a, float const b, float const duration)
{
  gasAnimation* animation = calloc(1, sizeof(_gasAnimation));
  animation->state = GAS_ANIMATION_STATE_NOT_STARTED;
  animation->type = GAS_ANIMATION_TYPE_NUMBER;
  animation->numberAnimation.type = type;
  animation->numberAnimation.target = target;
  animation->numberAnimation.a = a;
  animation->numberAnimation.b = b;
  animation->numberAnimation.duration = duration;
  animation->numberAnimation.easing = easing;
  animation->numberAnimation.time = 0.0f;
  return animation;
}

gasBoolean gasAnimate(gasAnimation* animation, glhckObject* object, float const delta)
{
  _gasAnimate(animation, object, delta);
  return animation->state != GAS_ANIMATION_STATE_FINISHED ? GAS_TRUE : GAS_FALSE;
}

float _gasAnimate(gasAnimation* animation, glhckObject* object, float const delta)
{
  if (animation->state == GAS_ANIMATION_STATE_FINISHED)
    return delta;

  switch (animation->type)
  {
    case GAS_ANIMATION_TYPE_NUMBER: return _gasAnimateNumberAnimation(animation, object, delta); break;
    case GAS_ANIMATION_TYPE_PAUSE: return _gasAnimatePauseAnimation(animation, object, delta); break;
    case GAS_ANIMATION_TYPE_SEQUENTIAL: return _gasAnimateSequentialAnimation(animation, object, delta); break;
    case GAS_ANIMATION_TYPE_PARALLEL: return _gasAnimateParallelAnimation(animation, object, delta); break;
    default: assert(0);
  }
}

float _gasAnimateNumberAnimation(gasAnimation* animation, glhckObject* object, float const delta)
{
  if (animation->state == GAS_ANIMATION_STATE_NOT_STARTED)
  {
    animation->numberAnimation.time = 0.0f;

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
    default: assert(0);
  }
}


gasAnimation* gasSequentialAnimationNew(gasAnimation** children, const unsigned int numChildren)
{
  gasAnimation* animation = calloc(1, sizeof(_gasAnimation));
  animation->state = GAS_ANIMATION_STATE_NOT_STARTED;
  animation->type = GAS_ANIMATION_TYPE_SEQUENTIAL;
  animation->sequentialAnimation.numChildren = numChildren;
  animation->sequentialAnimation.currentIndex = 0;
  animation->sequentialAnimation.children = calloc(numChildren, sizeof(gasAnimation*));
  memcpy(animation->sequentialAnimation.children, children, numChildren * sizeof(gasAnimation*));

  return animation;
}


gasAnimation* gasPauseAnimationNew(const float duration)
{
  gasAnimation* animation = calloc(1, sizeof(_gasAnimation));
  animation->state = GAS_ANIMATION_STATE_NOT_STARTED;
  animation->type = GAS_ANIMATION_TYPE_SEQUENTIAL;
  animation->pauseAnimation.duration = duration;
  return animation;
}


gasAnimation* gasParallelAnimationNew(gasAnimation** children, const unsigned int numChildren)
{
  gasAnimation* animation = calloc(1, sizeof(_gasAnimation));
  animation->state = GAS_ANIMATION_STATE_NOT_STARTED;
  animation->type = GAS_ANIMATION_TYPE_PARALLEL;
  animation->parallelAnimation.numChildren = numChildren;
  animation->parallelAnimation.children = calloc(numChildren, sizeof(gasAnimation*));
  memcpy(animation->parallelAnimation.children, children, numChildren * sizeof(gasAnimation*));
  return animation;
}
