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

gasAnimation* gasNumberAnimationFromToNew(gasNumberAnimationTarget const target, gasEasingType const easing,
                                          float const from, float const to, float const duration);
gasAnimation* gasNumberAnimationFromDeltaNew(gasNumberAnimationTarget const target, gasEasingType const easing,
                                             float const from, float const delta, float const duration);
gasAnimation* gasNumberAnimationDeltaToNew(gasNumberAnimationTarget const target, gasEasingType const easing,
                                           float const delta, float const to, float const duration);
gasAnimation* gasNumberAnimationFromNew(gasNumberAnimationTarget const target, gasEasingType const easing,
                                        float const from, float const duration);
gasAnimation* gasNumberAnimationToNew(gasNumberAnimationTarget const target, gasEasingType const easing,
                                      float const to, float const duration);
gasAnimation* gasNumberAnimationDeltaNew(gasNumberAnimationTarget const target, gasEasingType const easing,
                                         float const delta, float const duration);

void gasAnimationFree(gasAnimation* animation);

gasBoolean gasAnimate(gasAnimation* animation, glhckObject* object, float const delta);

#ifdef __cplusplus
}
#endif

#endif
