#include "GLFW/glfw3.h"
#include "glhck/glhck.h"
#include "gas.h"

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 800
#define HEIGHT 480


int main(int argc, char** argv)
{
  if (!glfwInit())
    return EXIT_FAILURE;

  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "gas-test-compound", NULL, NULL);
  glfwMakeContextCurrent(window);

  if(!window)
  {
    return EXIT_FAILURE;
  }

  glfwSwapInterval(1);

  if(!glhckContextCreate(argc, argv))
  {
    printf("GLHCK initialization error\n");
    return EXIT_FAILURE;
  }

  glhckLogColor(0);
  if(!glhckDisplayCreate(WIDTH, HEIGHT, GLHCK_RENDER_AUTO))
  {
    printf("GLHCK display create error");
    return EXIT_FAILURE;
  }

  glhckRenderClearColorb(128, 128, 128, 255);

  glhckObject* cube1 = glhckCubeNew(10);
  glhckObjectPositionf(cube1, 0, HEIGHT/2, 0);
  gasAnimation* animation1 = gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_X, GAS_EASING_LINEAR, 0.0f, 400.0f, 1.0f);
  gasAnimation* animation2 = gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_Y, GAS_EASING_QUAD_OUT, 240.0f, 0.0f, 1.0f);
  gasAnimation* animation3 = gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_Y, GAS_EASING_QUAD_IN, 0.0f, 240.0f, 1.0f);
  gasAnimation* animation4 = gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_X, GAS_EASING_LINEAR, 400.0f, 800.0f, 1.0f);
  gasAnimation* animations[] = { animation1, animation2, animation3, animation4 };
  gasAnimation* animation1234 = gasSequentialAnimationNew(animations, 4);

  glhckObject* cube2 = glhckCubeNew(10);
  glhckObjectPositionf(cube2, WIDTH/2 + 100, HEIGHT/2, 0);
  gasAnimation* topRight[] = {
    gasNumberAnimationNewDelta(GAS_NUMBER_ANIMATION_TARGET_X, GAS_EASING_QUAD_IN, -100.0f, 1.0f),
    gasNumberAnimationNewDelta(GAS_NUMBER_ANIMATION_TARGET_Y, GAS_EASING_QUAD_OUT, -100.0f, 1.0f)
  };
  gasAnimation* topLeft[] = {
    gasNumberAnimationNewDelta(GAS_NUMBER_ANIMATION_TARGET_X, GAS_EASING_QUAD_OUT, -100.0f, 1.0f),
    gasNumberAnimationNewDelta(GAS_NUMBER_ANIMATION_TARGET_Y, GAS_EASING_QUAD_IN, 100.0f, 1.0f)
  };
  gasAnimation* bottomLeft[] = {
    gasNumberAnimationNewDelta(GAS_NUMBER_ANIMATION_TARGET_X, GAS_EASING_QUAD_IN, 100.0f, 1.0f),
    gasNumberAnimationNewDelta(GAS_NUMBER_ANIMATION_TARGET_Y, GAS_EASING_QUAD_OUT, 100.0f, 1.0f)
  };
  gasAnimation* bottomRight[] = {
    gasNumberAnimationNewDelta(GAS_NUMBER_ANIMATION_TARGET_X, GAS_EASING_QUAD_OUT, 100.0f, 1.0f),
    gasNumberAnimationNewDelta(GAS_NUMBER_ANIMATION_TARGET_Y, GAS_EASING_QUAD_IN, -100.0f, 1.0f)
  };

  gasAnimation* circleParts[] = {
    gasParallelAnimationNew(topRight, 2),
    gasParallelAnimationNew(topLeft, 2),
    gasParallelAnimationNew(bottomLeft, 2),
    gasParallelAnimationNew(bottomRight, 2)
  };

  gasAnimation* circle = gasSequentialAnimationNew(circleParts, 4);

  float time = glfwGetTime();
  while(time < 6)
  {
    float oldTime = time;
    time = glfwGetTime();
    float delta = time - oldTime;

    glfwPollEvents();

    gasAnimate(animation1234, cube1, delta);
    gasAnimate(circle, cube2, delta);

    glhckRenderClear(GLHCK_DEPTH_BUFFER_BIT | GLHCK_COLOR_BUFFER_BIT);

    glhckObjectDraw(cube1);
    glhckObjectDraw(cube2);

    glhckRender();
    glfwSwapBuffers(window);
  }

  gasAnimationFree(animation1234);
  glhckObjectFree(cube1);

  gasAnimationFree(circle);
  glhckObjectFree(cube2);

  glhckContextTerminate();
  glfwTerminate();
  return EXIT_SUCCESS;
}
