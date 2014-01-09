#ifndef GASXX_H
#define GASXX_H

#include "gas.h"
#include <vector>
#include <string>

namespace gas
{
  class Animation
  {
  public:
    static Animation fromTo(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                  float const from, float const to, float const duration)
    {
      return Animation(gasNumberAnimationNewFromTo(target, easing, from, to, duration));
    }

    static Animation fromDelta(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                     float const from, float const delta, float const duration)
    {
      return Animation(gasNumberAnimationNewFromDelta(target, easing, from, delta, duration));
    }

    static Animation deltaTo(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                   float const delta, float const to, float const duration)
    {
      return Animation(gasNumberAnimationNewDeltaTo(target, easing, delta, to, duration));
    }

    static Animation from(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                float const from, float const duration)
    {
      return Animation(gasNumberAnimationNewFrom(target, easing, from, duration));
    }

    static Animation to(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                float const to, float const duration)
    {
      return Animation(gasNumberAnimationNewTo(target, easing, to, duration));
    }

    static Animation delta(gasNumberAnimationTarget const target, gasEasingFunc const easing,
                                float const delta, float const duration)
    {
      return Animation(gasNumberAnimationNewDelta(target, easing, delta, duration));
    }

    static Animation pause(float const duration)
    {
      return Animation(gasPauseAnimationNew(duration));
    }

    static Animation sequential(std::vector<Animation>&& children)
    {
      std::vector<gasAnimation*> animations;
      for(Animation& child : children)
      {
        animations.push_back(child.animation);
        child.animation = nullptr;
      }
      return Animation(gasSequentialAnimationNew(animations.data(), animations.size()));
    }
    static Animation parallel(std::vector<Animation>&& children)
    {
      std::vector<gasAnimation*> animations;
      for(Animation& child : children)
      {
        animations.push_back(child.animation);
        child.animation = nullptr;
      }
      return Animation(gasParallelAnimationNew(animations.data(), animations.size()));
    }
    static Animation model(std::string const& name, float duration)
    {
      return Animation(gasModelAnimationNew(name.data(), duration));
    }
    static Animation action(gasActionCallback callback, void* userdata = nullptr, gasActionResetCallback resetCallback = nullptr,
                            gasActionCloneCallback cloneCallback = nullptr, gasActionFreeCallback freeCallback = nullptr)
    {
      return Animation(gasActionNew(callback, resetCallback, cloneCallback, freeCallback, userdata));
    }
    static Animation custom(gasCustomAnimationCallback callback, void* userdata = nullptr, gasCustomAnimationResetCallback resetCallback = nullptr,
                            gasCustomAnimationCloneCallback cloneCallback = nullptr, gasCustomAnimationFreeCallback freeCallback = nullptr)
    {
      return Animation(gasCustomAnimationNew(callback, resetCallback, cloneCallback, freeCallback, userdata));
    }

    static const Animation NONE;

    ~Animation()
    {
      freeAnimation();
    }

    Animation(Animation const& other) : animation(nullptr)
    {
      if(other.animation != nullptr)
      {
        animation = gasAnimationClone(other.animation);
      }
    }

    Animation(Animation&& other) : animation(other.animation)
    {
      other.animation = nullptr;
    }

    Animation& operator=(Animation const& other)
    {
      if(&other != this)
      {
        freeAnimation();
        if(other.animation != nullptr)
        {
          std::cout << "cloning animation " << ++n << std::endl;
          animation = gasAnimationClone(other.animation);
        }
        else
        {
          animation = nullptr;
        }
        return *this;
      }
    }

    Animation& operator=(Animation&& other)
    {
      if(&other != this)
      {
        freeAnimation();
        animation = other.animation;
        other.animation = nullptr;
        return *this;
      }
    }

    operator bool()
    {
      return animation != nullptr;
    }

    bool animate(glhckObject* object, float const delta)
    {
      return animation == nullptr || gasAnimate(animation, object, delta);
    }

    gasAnimationState getState()
    {
      return animation == nullptr ? GAS_ANIMATION_STATE_NOT_STARTED : gasAnimationGetState(animation);
    }

    Animation& loop(unsigned int times)
    {
      if(animation != nullptr)
      {
        gasAnimationLoopTimes(animation, times);
      }
      return *this;
    }

    Animation& loop()
    {
      if(animation != nullptr)
      {
        gasAnimationLoop(animation);
      }
      return *this;
    }

    void reset()
    {
      if(animation != nullptr)
      {
        gasAnimationReset(animation);
      }
    }

  protected:
    Animation(gasAnimation* animation) : animation(animation) {}
    static int n;
    void freeAnimation()
    {
      if(animation != nullptr)
      {
        gasAnimationFree(animation);
        animation = nullptr;
      }
    }

    gasAnimation* animation;
  };

   Animation const Animation::NONE = Animation(nullptr);
   int Animation::n = 0;
}


#endif // GASXX_H
