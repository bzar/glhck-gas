#include "gas.h"
#include "internal.h"

#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

gasAnimation* gasNumberAnimationNewFromTo(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                          float const from, float const to, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_FROM_TO, from, to, duration);
}

gasAnimation* gasNumberAnimationNewFromDelta(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                             float const from, float const delta, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_FROM_DELTA, from, delta, duration);
}

gasAnimation* gasNumberAnimationNewDeltaTo(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                           float const delta, float const to, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_DELTA_TO, delta, to, duration);
}

gasAnimation* gasNumberAnimationNewFrom(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                        float const from, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_FROM, from, 0.0f, duration);
}

gasAnimation* gasNumberAnimationNewTo(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                      float const to, float const duration)
{
  return _gasNumberAnimationNew(target, easing, GAS_NUMBER_ANIMATION_TYPE_TO, 0.0f, to, duration);
}

gasAnimation* gasNumberAnimationNewDelta(gasNumberAnimationTarget const target, gasEasingFunc easing,
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
      if (animation->modelAnimation.animator)
      {
        glhckAnimatorFree(animation->modelAnimation.animator);
      }
      free(animation->modelAnimation.name);
      free(animation);
      break;
    }
    case GAS_ANIMATION_TYPE_ACTION:
    {
      if(animation->action.freeCallback)
      {
        animation->action.freeCallback(animation->action.userdata);
      }
      free(animation);
      break;
    }
    case GAS_ANIMATION_TYPE_CUSTOM:
    {
      if(animation->customAnimation.freeCallback)
      {
        animation->customAnimation.freeCallback(animation->customAnimation.userdata);
      }
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

gasAnimation* gasModelAnimationNew(const char* name, float duration)
{
  gasAnimation* animation = _gasAnimationNew(GAS_ANIMATION_TYPE_MODEL);
  animation->modelAnimation.name = strdup(name);
  animation->modelAnimation.animator = NULL;
  animation->modelAnimation.duration = duration;

  return animation;
}

gasAnimation* gasActionNew(gasActionCallback callback, gasActionResetCallback resetCallback,
                           gasActionCloneCallback cloneCallback, gasActionFreeCallback freeCallback, void* userdata)
{
  gasAnimation* animation = _gasAnimationNew(GAS_ANIMATION_TYPE_ACTION);
  animation->action.callback = callback;
  animation->action.resetCallback = resetCallback;
  animation->action.cloneCallback = cloneCallback;
  animation->action.freeCallback = freeCallback;
  animation->action.userdata = userdata;
  return animation;
}

gasAnimation* gasCustomAnimationNew(gasCustomAnimationCallback callback, gasCustomAnimationResetCallback resetCallback,
                                    gasCustomAnimationCloneCallback cloneCallback, gasCustomAnimationFreeCallback freeCallback,
                                    void* userdata)
{
  gasAnimation* animation = _gasAnimationNew(GAS_ANIMATION_TYPE_CUSTOM);
  animation->customAnimation.callback = callback;
  animation->customAnimation.resetCallback = resetCallback;
  animation->customAnimation.cloneCallback = cloneCallback;
  animation->customAnimation.freeCallback = freeCallback;
  animation->customAnimation.userdata = userdata;
  return animation;
}

gasBoolean gasAnimate(gasAnimation* animation, glhckObject* object, float const delta)
{
  _gasAnimate(animation, object, delta);
  return animation->state != GAS_ANIMATION_STATE_FINISHED ? GAS_TRUE : GAS_FALSE;
}

gasAnimation*  gasAnimationLoopTimes(gasAnimation* animation, unsigned int times)
{
  animation->loops = times;
  return animation;
}


gasAnimation* gasAnimationLoop(gasAnimation* animation)
{
  animation->loops = -1;
  return animation;
}


void gasAnimationReset(gasAnimation* animation)
{
  animation->loop = 0;
  _gasAnimationResetCurrentLoop(animation);
}

gasManager* gasManagerNew()
{
  gasManager* manager = calloc(1, sizeof(_gasManager));
  manager->animations = NULL;
  manager->newAnimations = NULL;
  manager->removeAnimations = NULL;
  return manager;
}


void gasManagerFree(gasManager* manager)
{
  while (manager->animations)
  {
    manager->animations = _gasManagerAnimationFree(manager->animations);
  }

  while (manager->newAnimations)
  {
    manager->newAnimations = _gasManagerAnimationFree(manager->newAnimations);
  }

  while (manager->removeAnimations)
  {
    _gasManagerAnimationReference* ref = manager->removeAnimations;
    manager->removeAnimations = ref->next;
    free(ref);
  }

  free(manager);
}


void gasManagerAddAnimation(gasManager* manager, gasAnimation* animation, glhckObject* object)
{
  _gasManagerAnimation* a = _gasManagerAnimationNew(animation, object);
  a->next = manager->newAnimations;
  manager->newAnimations = a;
}


void gasManagerRemoveAnimation(gasManager* manager, gasAnimation* animation)
{
  _gasManagerAnimation** a = &manager->animations;
  while (*a && (*a)->animation != animation)
  {
    a = &(*a)->next;
  }

  if (*a)
  {
    _gasManagerEnqueueRemoveAnimation(manager, *a);
  }
}


void gasManagerRemoveObjectAnimations(gasManager* manager, glhckObject* object)
{
  _gasManagerAnimation** a = &manager->animations;
  while (*a)
  {
    if ((*a)->object == object)
    {
      _gasManagerEnqueueRemoveAnimation(manager, *a);
    }

    a = &(*a)->next;
  }
}


void gasManagerAnimate(gasManager* manager, const float delta)
{
  while (manager->removeAnimations)
  {
    _gasManagerAnimationReference* ref = manager->removeAnimations;
    manager->removeAnimations = ref->next;
    _gasManagerRemoveAnimationByReference(manager, ref);
    free(ref);
  }

  while (manager->newAnimations)
  {
    _gasManagerAnimation* a = manager->newAnimations;
    manager->newAnimations = a->next;
    a->next = manager->animations;
    manager->animations = a;
  }

  _gasManagerAnimation** a = &manager->animations;
  while (*a)
  {
    if (gasAnimate((*a)->animation, (*a)->object, delta))
    {
      a = &(*a)->next;
    }
    else
    {
      *a = _gasManagerAnimationFree(*a);
    }
  }
}


float gasEasingLinear(float t)
{
  return t;
}

float gasEasingQuadIn(float t)
{
  return t * t;
}

float gasEasingQuadOut(float t)
{
  return 2 * t - t * t;
}

float gasEasingEase(float t)
{
  return gasEasingCubicBezier(t, 0.25, 0.1, 0.25, 1);
}

float gasEasingEaseIn(float t)
{
  return gasEasingCubicBezier(t, 0.42, 0, 1, 1);
}

float gasEasingEaseOut(float t)
{
  return gasEasingCubicBezier(t, 0, 0, 0.58, 1);
}

float gasEasingEaseInOut(float t)
{
  return gasEasingCubicBezier(t, 0.42, 0, 0.58, 1);
}

float gasEasingCubicBezier(float x, float x1, float y1, float x2, float y2)
{
  if (x <= 0) return 0;
  if (x >= 1) return 1;
  float const t = _gasCubicBezierTFromX(x, x1, x2);
  return _gasCubicBezierYFromT(t, y1, y2);
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

gasAnimation* _gasNumberAnimationNew(gasNumberAnimationTarget const target, gasEasingFunc easing,
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
      case GAS_ANIMATION_TYPE_ACTION: left = _gasAnimateAction(animation, object, delta); break;
      case GAS_ANIMATION_TYPE_CUSTOM: left = _gasAnimateCustomAnimation(animation, object, delta); break;
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

  float const relativeTime = animation->numberAnimation.duration > 0.0f
      ? animation->numberAnimation.time / animation->numberAnimation.duration
      : 1.0f;

  animation->state = animation->numberAnimation.time >= animation->numberAnimation.duration
      ? GAS_ANIMATION_STATE_FINISHED
      : GAS_ANIMATION_STATE_RUNNING;

  float const t = animation->numberAnimation.easing(_gasClamp(relativeTime, 0, 1));

  float value;
  switch (animation->numberAnimation.type)
  {
    case GAS_NUMBER_ANIMATION_TYPE_FROM_TO:
    case GAS_NUMBER_ANIMATION_TYPE_FROM:
    case GAS_NUMBER_ANIMATION_TYPE_TO:
    {
      value = animation->numberAnimation.a + (animation->numberAnimation.b - animation->numberAnimation.a) * t;
      break;
    }
    case GAS_NUMBER_ANIMATION_TYPE_FROM_DELTA:
    case GAS_NUMBER_ANIMATION_TYPE_DELTA:
    {
      value = animation->numberAnimation.a + animation->numberAnimation.b * t;
      break;
    }
    case GAS_NUMBER_ANIMATION_TYPE_DELTA_TO:
    {
      value = (animation->numberAnimation.b - animation->numberAnimation.a) + animation->numberAnimation.a * t;
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

  if(animation->modelAnimation.animator == NULL)
  {
    glhckAnimator* animator = glhckAnimatorNew();

    unsigned int numAnimations;
    glhckAnimation** animations = glhckObjectAnimations(object, &numAnimations);
    glhckAnimation* modelAnimation = NULL;
    int i;
    for(i = 0; i < numAnimations; ++i)
    {
      if(strcmp(animation->modelAnimation.name, glhckAnimationGetName(animations[i])) == 0)
      {
        modelAnimation = animations[i];
        break;
      }
    }

    assert(modelAnimation);
    glhckAnimatorAnimation(animator, modelAnimation);
    animation->modelAnimation.animationDuration = glhckAnimationGetDuration(modelAnimation);

    unsigned int numBones;
    glhckBone** bones = glhckObjectBones(object, &numBones);
    glhckAnimatorInsertBones(animator, bones, numBones);

    animation->modelAnimation.animator = animator;

  }
  animation->modelAnimation.time += delta;
  float position = animation->modelAnimation.time / animation->modelAnimation.duration;
  glhckAnimatorUpdate(animation->modelAnimation.animator, _gasClamp(position, 0.0f, 1.0f) * animation->modelAnimation.animationDuration);
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

float _gasAnimateAction(gasAnimation* animation, glhckObject* object, float const delta)
{
  if(animation->action.callback)
  {
    animation->action.callback(object, animation->action.userdata);
  }

  animation->state = GAS_ANIMATION_STATE_FINISHED;
  return delta;
}

float _gasAnimateCustomAnimation(gasAnimation* animation, glhckObject* object, float const delta)
{
  float left = delta;
  if(animation->customAnimation.callback)
  {
    left = animation->customAnimation.callback(object, delta, animation->customAnimation.userdata);
  }

  animation->state = left > 0
      ? GAS_ANIMATION_STATE_FINISHED
      : GAS_ANIMATION_STATE_RUNNING;

  return left;
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
    case GAS_ANIMATION_TYPE_ACTION: return _gasAnimationResetAction(animation); break;
    case GAS_ANIMATION_TYPE_CUSTOM: return _gasAnimationResetCustomAnimation(animation); break;
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

void _gasAnimationResetAction(gasAnimation* animation)
{
  if(animation->action.resetCallback)
  {
    animation->action.resetCallback(animation->action.userdata);
  }
}

void _gasAnimationResetCustomAnimation(gasAnimation* animation)
{
  if(animation->customAnimation.resetCallback)
  {
    animation->customAnimation.resetCallback(animation->customAnimation.userdata);
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

  switch (animation->type)
  {
    case GAS_ANIMATION_TYPE_NUMBER: break;
    case GAS_ANIMATION_TYPE_PAUSE: break;
    case GAS_ANIMATION_TYPE_SEQUENTIAL: break;
    case GAS_ANIMATION_TYPE_PARALLEL: break;
    case GAS_ANIMATION_TYPE_MODEL: break;
    case GAS_ANIMATION_TYPE_ACTION:
    {
      if(animation->action.cloneCallback)
      {
        newAnimation->action.userdata = animation->action.cloneCallback(animation->action.userdata);
      }
      break;
    }
    case GAS_ANIMATION_TYPE_CUSTOM: break;
    {
      if(animation->customAnimation.cloneCallback)
      {
        newAnimation->customAnimation.userdata = animation->customAnimation.cloneCallback(animation->customAnimation.userdata);
      }
      break;
    }
    default: assert(0);
  }

  return newAnimation;
}

_gasManagerAnimation* _gasManagerAnimationNew(gasAnimation* animation, glhckObject* object)
{
  _gasManagerAnimation* a = calloc(1, sizeof(_gasManagerAnimation));
  a->animation = animation;
  a->object = object;
  a->manageObject = GAS_FALSE;
  a->next = NULL;
  return a;
}

_gasManagerAnimation* _gasManagerAnimationFree(_gasManagerAnimation* animation)
{
  _gasManagerAnimation* next = animation->next;
  gasAnimationFree(animation->animation);
  free(animation);
  return next;
}

_gasManagerAnimationReference* _gasManagerEnqueueRemoveAnimation(_gasManager* manager, _gasManagerAnimation* animation)
{
  _gasManagerAnimationReference* ref = calloc(1, sizeof(_gasManagerAnimationReference));
  ref->animation = animation;
  ref->next = manager->removeAnimations;
  manager->removeAnimations = ref;
}

_gasManagerAnimationReference* _gasManagerRemoveAnimationByReference(_gasManager* manager, _gasManagerAnimationReference* ref)
{
  _gasManagerAnimation** a = &manager->animations;
  while (*a && *a != ref->animation)
  {
    a = &(*a)->next;
  }

  if (*a)
  {
    *a = _gasManagerAnimationFree(*a);
  }
}

float _gasCubicBezierXFromT(float t, float x1, float x2) {
  return 3 * (1-t) * (1-t) * t * x1 + 3 * (1-t) * t * t * x2 + t * t * t;
}

float _gasCubicBezierYFromT(float t, float y1, float y2) {
  return 3 * (1-t) * (1-t) * t * y1 + 3 * (1-t) * t * t * y2 + t * t * t;
}

float _gasCubicBezierTFromX(float x, float x1, float x2) {
  float mint = 0;
  float maxt = 1;
  int i;
  for (i = 0; i < 30; ++i)
  {
    float guesst = (mint + maxt) / 2;
    float guessx = _gasCubicBezierXFromT(guesst, x1, x2);
    if (x < guessx)
      maxt = guesst;
    else
      mint = guesst;
  }
  return (mint + maxt) / 2;
}
