#ifndef GASXX_H
#define GASXX_H

#include "gas.h"
#include <vector>
#include <string>
#include <functional>

namespace gas
{
  gasAnimation* numberAnimationNewFromTo(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                            float const from, float const to, float const duration)
  {
    return gasNumberAnimationNewFromTo(target, easing, from, to, duration);
  }

  gasAnimation* numberAnimationNewFromDelta(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                               float const from, float const delta, float const duration)
  {
    return gasNumberAnimationNewFromDelta(target, easing, from, delta, duration);
  }

  gasAnimation* numberAnimationNewDeltaTo(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                             float const delta, float const to, float const duration)
  {
    return gasNumberAnimationNewDeltaTo(target, easing, delta, to, duration);
  }

  gasAnimation* numberAnimationNewFrom(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                          float const from, float const duration)
  {
    return gasNumberAnimationNewFrom(target, easing, from, duration);
  }

  gasAnimation* numberAnimationNewTo(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                        float const to, float const duration)
  {
    return gasNumberAnimationNewTo(target, easing, to, duration);
  }

  gasAnimation* numberAnimationNewDelta(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                           float const delta, float const duration)
  {
    return gasNumberAnimationNewDelta(target, easing, delta, duration);
  }

  gasAnimation* pauseAnimationNew(float const duration)
  {
    return gasPauseAnimationNew(duration);
  }

  gasAnimation* sequentialAnimationNew(std::vector<gasAnimation*>&& children)
  {
    return gasSequentialAnimationNew(children.data(), children.size());
  }

  gasAnimation* parallelAnimationNew(std::vector<gasAnimation*>&& children)
  {
    return gasParallelAnimationNew(children.data(), children.size());
  }

  gasAnimation* modelAnimationNew(std::string const& name, float duration)
  {
    return gasModelAnimationNew(name.data(), duration);
  }

  gasAnimation* actionNew(gasActionCallback callback, gasActionResetCallback resetCallback,
                             gasActionCloneCallback cloneCallback, gasActionFreeCallback freeCallback, void* userdata)
  {
    return gasActionNew(callback, resetCallback, cloneCallback, freeCallback, userdata);
  }

  gasAnimation* actionNew(gasActionCallback callback, void* userdata)
  {
    return gasActionNew(callback, nullptr, nullptr, nullptr, userdata);
  }

  gasAnimation* customAnimationNew(gasCustomAnimationCallback callback, gasCustomAnimationResetCallback resetCallback,
                                      gasCustomAnimationCloneCallback cloneCallback, gasCustomAnimationFreeCallback freeCallback,
                                      void* userdata)
  {
    return gasCustomAnimationNew(callback, resetCallback, cloneCallback, freeCallback, userdata);
  }

  gasAnimation* animationClone(gasAnimation* animation)
  {
    return gasAnimationClone(animation);
  }

  void animationFree(gasAnimation* animation)
  {
    gasAnimationFree(animation);
  }

  bool animate(gasAnimation* animation, glhckObject* object, float const delta)
  {
    return gasAnimate(animation, object, delta);
  }

  gasAnimationState animationGetState(gasAnimation* animation)
  {
    return gasAnimationGetState(animation);
  }

  gasAnimation*  animationLoop(gasAnimation* animation, unsigned int times)
  {
    return gasAnimationLoopTimes(animation, times);
  }

  gasAnimation*  animationLoop(gasAnimation* animation)
  {
    return gasAnimationLoop(animation);
  }

  void animationReset(gasAnimation* animation)
  {
    gasAnimationReset(animation);
  }
}

#endif // GASXX_H
