#ifndef GAS_INTERNAL_H
#define GAS_INTERNAL_H

#include "gas.h"

typedef enum _gasAnimationType {
  GAS_ANIMATION_TYPE_NUMBER,
  GAS_ANIMATION_TYPE_PAUSE,
  GAS_ANIMATION_TYPE_SEQUENTIAL,
  GAS_ANIMATION_TYPE_PARALLEL
} _gasAnimationType;

typedef enum _gasAnimationState {
  GAS_ANIMATION_STATE_NOT_STARTED,
  GAS_ANIMATION_STATE_RUNNING,
  GAS_ANIMATION_STATE_FINISHED
} _gasAnimationState;

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
  gasEasingType easing;
  float time;
} _gasNumberAnimation;

typedef struct _gasPauseAnimation {
  float duration;
  float time;
} _gasPauseAnimation;

typedef struct _gasSequentialAnimation {
  struct _gasAnimation** children;
  unsigned int numChildren;
} _gasSequentialAnimation;

typedef struct _gasParallelAnimation {
  struct _gasAnimation** children;
  unsigned int numChildren;
} _gasParallelAnimation;

typedef struct _gasAnimation {
  _gasAnimationType type;
  _gasAnimationState state;

  union {
    _gasNumberAnimation numberAnimation;
    _gasPauseAnimation pauseAnimation;
    _gasSequentialAnimation sequentialAnimation;
    _gasParallelAnimation parallelAnimation;
  };
} _gasAnimation;

gasAnimation* _gasNumberAnimationNew(gasNumberAnimationTarget const target, gasEasingType const easing, _gasNumberAnimationType const type, float const a, float const b, float const duration);

float _gasAnimate(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimateNumberAnimation(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimatePauseAnimation(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimateSequentialAnimation(gasAnimation* animation, glhckObject* object, float const delta);
float _gasAnimateParallelAnimation(gasAnimation* animation, glhckObject* object, float const delta);

float _gasNumberAnimationGetTargetValue(gasNumberAnimationTarget target, glhckObject* object);
void _gasNumberAnimationSetTargetValue(gasNumberAnimationTarget target, glhckObject* object, float const value);

float _gasClamp(float const value, float const minValue, float const maxValue);

#endif // GAS_INTERNAL_H
