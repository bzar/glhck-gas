#ifndef GAS_INTERNAL_H
#define GAS_INTERNAL_H

#include "gas.h"

typedef enum _gasAnimationType {
  GAS_ANIMATION_TYPE_NUMBER,
  GAS_ANIMATION_TYPE_PAUSE,
  GAS_ANIMATION_TYPE_SEQUENTIAL,
  GAS_ANIMATION_TYPE_PARALLEL,
  GAS_ANIMATION_TYPE_MODEL,
  GAS_ANIMATION_TYPE_ACTION,
  GAS_ANIMATION_TYPE_CUSTOM
} _gasAnimationType;

typedef enum _gasNumberAnimationType {
  GAS_NUMBER_ANIMATION_TYPE_FROM_TO,
  GAS_NUMBER_ANIMATION_TYPE_FROM_DELTA,
  GAS_NUMBER_ANIMATION_TYPE_DELTA_TO,
  GAS_NUMBER_ANIMATION_TYPE_FROM,
  GAS_NUMBER_ANIMATION_TYPE_TO,
  GAS_NUMBER_ANIMATION_TYPE_DELTA
} _gasNumberAnimationType;

typedef struct _gasNumberAnimation {
  _gasNumberAnimationType type;
  gasNumberAnimationTarget target;
  float a;
  float b;
  float duration;
  gasEasingFunc easing;
  float time;
} _gasNumberAnimation;

typedef struct _gasPauseAnimation {
  float duration;
  float time;
} _gasPauseAnimation;

typedef struct _gasSequentialAnimation {
  struct _gasAnimation** children;
  unsigned int numChildren;
  unsigned int currentIndex;
} _gasSequentialAnimation;

typedef struct _gasParallelAnimation {
  struct _gasAnimation** children;
  unsigned int numChildren;
} _gasParallelAnimation;

typedef struct _gasModelAnimation {
  float duration;
  float time;
  char* name;
  glhckAnimator* animator;
  float animationDuration;
} _gasModelAnimation;

typedef struct _gasAction {
  gasActionCallback callback;
  gasActionResetCallback resetCallback;
  gasActionCloneCallback cloneCallback;
  gasActionFreeCallback freeCallback;
  void* userdata;
} _gasAction;

typedef struct _gasCustomAnimation {
  gasCustomAnimationCallback callback;
  gasCustomAnimationResetCallback resetCallback;
  gasCustomAnimationCloneCallback cloneCallback;
  gasCustomAnimationFreeCallback freeCallback;
  void* userdata;
} _gasCustomAnimation;

typedef struct _gasAnimation {
  _gasAnimationType type;
  gasAnimationState state;
  int loops;
  int loop;

  union {
    _gasNumberAnimation numberAnimation;
    _gasPauseAnimation pauseAnimation;
    _gasSequentialAnimation sequentialAnimation;
    _gasParallelAnimation parallelAnimation;
    _gasModelAnimation modelAnimation;
    _gasAction action;
    _gasCustomAnimation customAnimation;
  };
} _gasAnimation;

typedef struct _gasManagerAnimation
{
  glhckObject* object;
  gasAnimation* animation;
  gasBoolean manageObject;
  struct _gasManagerAnimation* next;
} _gasManagerAnimation;

typedef struct _gasManagerAnimationReference
{
  _gasManagerAnimation* animation;
  struct _gasManagerAnimationReference* next;
} _gasManagerAnimationReference;

typedef struct _gasManager
{
  _gasManagerAnimation* animations;
  _gasManagerAnimation* newAnimations;
  _gasManagerAnimationReference* removeAnimations;
} _gasManager;

gasAnimation* _gasAnimationNew(_gasAnimationType type);
gasAnimation* _gasNumberAnimationNew(gasNumberAnimationTarget const target, gasEasingFunc const easing, _gasNumberAnimationType const type, float const a, float const b, float const duration);

float _gasAnimate(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimateNumberAnimation(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimatePauseAnimation(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimateSequentialAnimation(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimateParallelAnimation(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimateModelAnimation(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimateAction(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimateCustomAnimation(gasAnimation* animation, glhckObject* object, float const delta);

void _gasAnimationResetCurrentLoop(gasAnimation* animation);
void _gasAnimationResetNumberAnimation(gasAnimation* animation);
void _gasAnimationResetPauseAnimation(gasAnimation* animation);
void _gasAnimationResetSequentialAnimation(gasAnimation* animation);
void _gasAnimationResetParallelAnimation(gasAnimation* animation);
void _gasAnimationResetModelAnimation(gasAnimation* animation);
void _gasAnimationResetAction(gasAnimation* animation);
void _gasAnimationResetCustomAnimation(gasAnimation* animation);

float _gasNumberAnimationGetTargetValue(gasNumberAnimationTarget target, glhckObject* object);
void _gasNumberAnimationSetTargetValue(gasNumberAnimationTarget target, glhckObject* object, float const value);

float _gasClamp(float const value, float const minValue, float const maxValue);
float _gasLoopsLeft(_gasAnimation* animation);

_gasManagerAnimation* _gasManagerAnimationNew(gasAnimation* animation, glhckObject* object);
_gasManagerAnimation* _gasManagerAnimationFree(_gasManagerAnimation* animation);
_gasManagerAnimationReference* _gasManagerEnqueueRemoveAnimation(_gasManager* manager, _gasManagerAnimation* animation);
_gasManagerAnimationReference* _gasManagerRemoveAnimationByReference(_gasManager* manager, _gasManagerAnimationReference* ref);

float _gasCubicBezierXFromT(float t, float x1, float x2);
float _gasCubicBezierYFromT(float t, float y1, float y2);
float _gasCubicBezierTFromX(float x, float x1, float x2);

#endif // GAS_INTERNAL_H
