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
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "gas-test-simple", NULL, NULL);
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
  gasAnimation* animation1 = gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_X, GAS_EASING_LINEAR, 0.0f, 800.0f, 5.0f);

  glhckObject* cube2 = glhckCubeNew(10);
  glhckObjectPositionf(cube2, WIDTH/2, 0, 0);
  gasAnimation* animation2 = gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_Y, GAS_EASING_QUAD_OUT, 0.0f, 480.0f, 4.0f);

  glhckObject* cube3 = glhckCubeNew(10);
  glhckObjectPositionf(cube3, WIDTH/4, HEIGHT/4, 0);
  gasAnimation* animation3 = gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_ROT_Z, GAS_EASING_QUAD_IN, 0.0f, 720.0f, 5.0f);

  float time = glfwGetTime();
  while(time < 6)
  {
    float oldTime = time;
    time = glfwGetTime();
    float delta = time - oldTime;

    glfwPollEvents();

    gasAnimate(animation1, cube1, delta);
    gasAnimate(animation2, cube2, delta);
    gasAnimate(animation3, cube3, delta);

    glhckRenderClear(GLHCK_DEPTH_BUFFER | GLHCK_COLOR_BUFFER);

    glhckObjectDraw(cube1);
    glhckObjectDraw(cube2);
    glhckObjectDraw(cube3);

    glhckRender();
    glfwSwapBuffers(window);
  }

  gasAnimationFree(animation1);
  gasAnimationFree(animation2);
  gasAnimationFree(animation3);
  glhckObjectFree(cube1);
  glhckObjectFree(cube2);
  glhckObjectFree(cube3);

  glhckContextTerminate();
  glfwTerminate();
  return EXIT_SUCCESS;
}
