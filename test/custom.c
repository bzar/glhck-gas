#include "GLFW/glfw3.h"
#include "glhck/glhck.h"
#include "gas.h"

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 800
#define HEIGHT 480


float changeOpacity(glhckObject* object, float delta, void* userdata)
{
  delta = delta > 0.1 ? 0.1 : delta;
  unsigned char* opacityTarget = (unsigned char*) userdata;
  glhckMaterial* material = glhckObjectGetMaterial(object);
  glhckColorb diffuse = *glhckMaterialGetDiffuse(material);

  float left = 0;
  if(diffuse.a < *opacityTarget)
  {
    if(*opacityTarget - diffuse.a < delta * 255)
    {
      diffuse.a = *opacityTarget;
      left = delta * (1 - (*opacityTarget - diffuse.a) / (delta * 255));
    }
    else
    {
      diffuse.a += delta * 255;
    }
  }
  else
  {
    if(diffuse.a - *opacityTarget < delta * 255)
    {
      diffuse.a = *opacityTarget;
      left = delta * (1 - (diffuse.a - *opacityTarget) / (delta * 255));
    }
    else
    {
      diffuse.a -= delta * 255;
    }
  }

  glhckMaterialDiffuse(material, &diffuse);

  return left;
}

void changeOpacityTarget(glhckObject* object, void* userdata)
{
  unsigned char* opacityTarget = (unsigned char*) userdata;
  *opacityTarget = *opacityTarget > 0 ? 0 : 255;
  printf("changeOpacityTarget to %d\n", *opacityTarget);
}

int main(int argc, char** argv)
{
  if (!glfwInit())
    return EXIT_FAILURE;

  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "gas-test-custom", NULL, NULL);
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
  glhckMaterial* cube1Material = glhckMaterialNew(NULL);
  glhckObjectMaterial(cube1, cube1Material);
  glhckMaterialFree(cube1Material);
  glhckMaterialBlendFunc(cube1Material, GLHCK_SRC_ALPHA, GLHCK_DST_ALPHA);
  glhckObjectPositionf(cube1,  WIDTH/2, HEIGHT/2, 0);

  unsigned char opacityTarget = 0;
  gasAnimation* animations[] = {
    gasCustomAnimationNew(changeOpacity, NULL, &opacityTarget),
    gasActionNew(changeOpacityTarget, NULL, &opacityTarget)
  };
  gasAnimation* animation1 = gasSequentialAnimationNew(animations, 2);
  gasAnimationLoopTimes(animation1, 5);

  float time = glfwGetTime();
  while(time < 6)
  {
    float oldTime = time;
    time = glfwGetTime();
    float delta = time - oldTime;

    glfwPollEvents();

    gasAnimate(animation1, cube1, delta);

    glhckRenderClear(GLHCK_DEPTH_BUFFER_BIT | GLHCK_COLOR_BUFFER_BIT);

    glhckObjectDraw(cube1);

    glhckRender();
    glfwSwapBuffers(window);
  }

  gasAnimationFree(animation1);
  glhckObjectFree(cube1);

  glhckContextTerminate();
  glfwTerminate();
  return EXIT_SUCCESS;
}
