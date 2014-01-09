#ifndef GAS_H
#define GAS_H

#include "glhck/glhck.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Enums */
typedef enum gasBoolean {
  GAS_FALSE = 0,
  GAS_TRUE = 1
} gasBoolean;

typedef enum gasAnimationState {
  GAS_ANIMATION_STATE_NOT_STARTED,
  GAS_ANIMATION_STATE_RUNNING,
  GAS_ANIMATION_STATE_FINISHED
} gasAnimationState;

typedef enum gasNumberAnimationTarget {
  GAS_NUMBER_ANIMATION_TARGET_X,
  GAS_NUMBER_ANIMATION_TARGET_Y,
  GAS_NUMBER_ANIMATION_TARGET_Z,
  GAS_NUMBER_ANIMATION_TARGET_ROT_X,
  GAS_NUMBER_ANIMATION_TARGET_ROT_Y,
  GAS_NUMBER_ANIMATION_TARGET_ROT_Z
} gasNumberAnimationTarget;

/* Callbacks */
typedef void (*gasActionCallback)(glhckObject* object, void* userdata);
typedef void (*gasActionResetCallback)(void* userdata);
typedef void* (*gasActionCloneCallback)(void* userdata);
typedef void (*gasActionFreeCallback)(void* userdata);

/* Returns seconds left over from delta, return value > 0 means the animation is finished */
typedef float (*gasCustomAnimationCallback)(glhckObject* object, float delta, void* userdata);
typedef void (*gasCustomAnimationResetCallback)(void* userdata);
typedef void* (*gasCustomAnimationCloneCallback)(void* userdata);
typedef void (*gasCustomAnimationFreeCallback)(void* userdata);

typedef float (*gasEasingFunc)(float t);

/* Types */
typedef struct _gasAnimation gasAnimation;
typedef struct _gasManager gasManager;


/* Animation */
gasAnimation* gasNumberAnimationNewFromTo(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                          float const from, float const to, float const duration);
gasAnimation* gasNumberAnimationNewFromDelta(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                             float const from, float const delta, float const duration);
gasAnimation* gasNumberAnimationNewDeltaTo(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                           float const delta, float const to, float const duration);
gasAnimation* gasNumberAnimationNewFrom(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                        float const from, float const duration);
gasAnimation* gasNumberAnimationNewTo(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                      float const to, float const duration);
gasAnimation* gasNumberAnimationNewDelta(gasNumberAnimationTarget const target, gasEasingFunc easing,
                                         float const delta, float const duration);

gasAnimation* gasPauseAnimationNew(float const duration);
gasAnimation* gasSequentialAnimationNew(gasAnimation** children, unsigned int const numChildren);
gasAnimation* gasParallelAnimationNew(gasAnimation** children, unsigned int const numChildren);
gasAnimation* gasModelAnimationNew(char const* name, float duration);
gasAnimation* gasActionNew(gasActionCallback callback, gasActionResetCallback resetCallback,
                           gasActionCloneCallback cloneCallback, gasActionFreeCallback freeCallback, void* userdata);
gasAnimation* gasCustomAnimationNew(gasCustomAnimationCallback callback, gasCustomAnimationResetCallback resetCallback,
                                    gasCustomAnimationCloneCallback cloneCallback, gasCustomAnimationFreeCallback freeCallback,
                                    void* userdata);
gasAnimation* gasAnimationClone(gasAnimation* animation);

void gasAnimationFree(gasAnimation* animation);

gasBoolean gasAnimate(gasAnimation* animation, glhckObject* object, float const delta);

gasAnimationState gasAnimationGetState(gasAnimation* animation);

gasAnimation*  gasAnimationLoopTimes(gasAnimation* animation, unsigned int times);
gasAnimation*  gasAnimationLoop(gasAnimation* animation);

void gasAnimationReset(gasAnimation* animation);

/* Manager */
gasManager* gasManagerNew();
void gasManagerFree(gasManager* manager);

void gasManagerAddAnimation(gasManager* manager, gasAnimation* animation, glhckObject* object);
void gasManagerRemoveAnimation(gasManager* manager, gasAnimation* animation);
void gasManagerRemoveObjectAnimations(gasManager* manager, glhckObject* object);
void gasManagerAnimate(gasManager* manager, float const delta);

/* Easing functions */

float gasEasingLinear(float t);
float gasEasingQuadIn(float t);
float gasEasingQuadOut(float t);
float gasEasingEase(float t);
float gasEasingEaseIn(float t);
float gasEasingEaseOut(float t);
float gasEasingEaseInOut(float t);

/* A general easing curve function to implement others with */
float gasEasingCubicBezier(float x, float p1x, float p1y, float p2x, float p2y);

#ifdef __cplusplus
}
#endif

#endif
