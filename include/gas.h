#ifndef GAS_H
#define GAS_H

#include "glhck/glhck.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum gasBoolean {
  GAS_FALSE = 0,
  GAS_TRUE = 1
} gasBoolean;

typedef enum gasNumberAnimationTarget {
  GAS_NUMBER_ANIMATION_TARGET_X,
  GAS_NUMBER_ANIMATION_TARGET_Y,
  GAS_NUMBER_ANIMATION_TARGET_Z,
  GAS_NUMBER_ANIMATION_TARGET_ROT_X,
  GAS_NUMBER_ANIMATION_TARGET_ROT_Y,
  GAS_NUMBER_ANIMATION_TARGET_ROT_Z
} gasNumberAnimationTarget;



typedef enum gasEasingType {
  GAS_EASING_LINEAR,
  GAS_EASING_QUAD_IN,
  GAS_EASING_QUAD_OUT
} gasEasingType;

typedef struct _gasAnimation gasAnimation;

gasAnimation* gasNumberAnimationNewFromTo(gasNumberAnimationTarget const target, gasEasingType const easing,
                                          float const from, float const to, float const duration);
gasAnimation* gasNumberAnimationNewFromDelta(gasNumberAnimationTarget const target, gasEasingType const easing,
                                             float const from, float const delta, float const duration);
gasAnimation* gasNumberAnimationNewDeltaTo(gasNumberAnimationTarget const target, gasEasingType const easing,
                                           float const delta, float const to, float const duration);
gasAnimation* gasNumberAnimationNewFrom(gasNumberAnimationTarget const target, gasEasingType const easing,
                                        float const from, float const duration);
gasAnimation* gasNumberAnimationNewTo(gasNumberAnimationTarget const target, gasEasingType const easing,
                                      float const to, float const duration);
gasAnimation* gasNumberAnimationNewDelta(gasNumberAnimationTarget const target, gasEasingType const easing,
                                         float const delta, float const duration);

gasAnimation* gasPauseAnimationNew(float const duration);
gasAnimation* gasSequentialAnimationNew(gasAnimation** children, unsigned int const numChildren);
gasAnimation* gasParallelAnimationNew(gasAnimation** children, unsigned int const numChildren);

void gasAnimationFree(gasAnimation* animation);

gasBoolean gasAnimate(gasAnimation* animation, glhckObject* object, float const delta);

void gasAnimationLoopTimes(gasAnimation* animation, unsigned int times);
void gasAnimationLoop(gasAnimation* animation);

void gasAnimationReset(gasAnimation* animation);

#ifdef __cplusplus
}
#endif

#endif
