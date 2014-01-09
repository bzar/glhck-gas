#include "GLFW/glfw3.h"
#include "glhck/glhck.h"
#include "gas.h"

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 800
#define HEIGHT 480

char RUNNING = 1;
void windowCloseCallback(GLFWwindow* window)
{
  RUNNING = 0;
}
void windowSizeCallback(GLFWwindow *window, int width, int height)
{
  glhckDisplayResize(width, height);
}

#define NUM_SHRAPNEL 64
#define NUM_INIT_ROCKETS 16
#define ROCKET_INTERVAL 2.0f
#define NUM_PARTICLES 1024

typedef struct Particle
{
  glhckObject* object;
  char alive;
} Particle;

Particle particles[NUM_PARTICLES] = {0};
gasManager* manager;

float blink(glhckObject* object, float delta, void* userdata)
{
  glhckMaterialDiffuseb(glhckObjectGetMaterial(object), rand()%256, rand()%256, rand()%256, 255);
  return 0;
}


void shrapnelDie(glhckObject* object, void* userdata)
{
  int i;
  Particle* p = NULL;
  for(i = 0; i < NUM_PARTICLES; ++i)
  {
    if(particles[i].object == object)
    {
      p = &particles[i];
      break;
    }
  }

  if(p)
  {
    p->alive = 0;
    gasManagerRemoveObjectAnimations(manager, p->object);
  }
}
gasAnimation* shrapnelAnimation(float dx, float dy, float duration)
{
  gasAnimation* a1[] = {
    gasNumberAnimationNewDelta(GAS_NUMBER_ANIMATION_TARGET_X, gasEasingQuadOut, dx, duration),
    gasNumberAnimationNewDelta(GAS_NUMBER_ANIMATION_TARGET_Y, gasEasingQuadOut, dy, duration),
  };

  gasAnimation* a2[] = {
    gasParallelAnimationNew(a1 , 2),
    gasActionNew(shrapnelDie, NULL, NULL, NULL, NULL)
  };
  return gasSequentialAnimationNew(a2, 2);
}

void rocketBoom(glhckObject* object, void* userdata)
{
  int i;
  Particle* p = NULL;
  for(i = 0; i < NUM_PARTICLES; ++i)
  {
    if(particles[i].object == object)
    {
      p = &particles[i];
      break;
    }
  }

  if(p)
  {
    p->alive = 0;
    const kmVec3* pos = glhckObjectGetPosition(p->object);
    int n = NUM_SHRAPNEL;
    for(i = 0; i < NUM_PARTICLES && n > 0; ++i)
    {
      if(!particles[i].alive)
      {
        particles[i].alive = 1;
        gasAnimation* a = shrapnelAnimation(rand() % 128 - 64, rand() % 128 - 64, 0.5f + (rand() % 10) / 10.0f);
        glhckObjectPosition(particles[i].object, pos);
        gasManagerAddAnimation(manager, a, particles[i].object);
        gasAnimation* blinkAnimation = gasCustomAnimationNew(blink, NULL, NULL, NULL, NULL);
        gasManagerAddAnimation(manager, blinkAnimation, particles[i].object);
        n -= 1;
      }
    }
  }
}

gasAnimation* rocketAnimation(float x, float y, float dx, float dy, float duration)
{
  gasAnimation* a1[] = {
    gasNumberAnimationNewFromDelta(GAS_NUMBER_ANIMATION_TARGET_X, gasEasingQuadIn, x, dx, duration),
    gasNumberAnimationNewFromDelta(GAS_NUMBER_ANIMATION_TARGET_Y, gasEasingLinear, y, dy, duration),
  };

  gasAnimation* a2[] = {
    gasParallelAnimationNew(a1 , 2),
    gasActionNew(rocketBoom, NULL, NULL, NULL, NULL)
  };
  return gasSequentialAnimationNew(a2 , 2);
}

void addRocket()
{
  int i;
  for(i = 0; i < NUM_PARTICLES; ++i)
  {
    if(!particles[i].alive)
    {
      gasAnimation* a = rocketAnimation(rand() % WIDTH, HEIGHT, rand() % 128 - 64, -HEIGHT/2 - rand() % (HEIGHT/2), 1.0f + (rand() % 50) / 10.0f);
      particles[i].alive = 1;
      gasManagerAddAnimation(manager, a, particles[i].object);
      break;
    }
  }
}

int main(int argc, char** argv)
{
  if (!glfwInit())
    return EXIT_FAILURE;

  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "gas-test-manager", NULL, NULL);
  glfwMakeContextCurrent(window);

  if(!window)
  {
    return EXIT_FAILURE;
  }

  glfwSetWindowCloseCallback(window, windowCloseCallback);
  glfwSetWindowSizeCallback(window, windowSizeCallback);
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

  glhckRenderClearColorb(32, 32, 32, 255);

  manager = gasManagerNew();

  int i;
  for(i = 0; i < NUM_PARTICLES; ++i)
  {
    particles[i].object = glhckCubeNew(4);
    glhckObjectMaterial(particles[i].object, glhckMaterialNew(NULL));
    particles[i].alive = 0;
  }

  for(i = 0; i < NUM_INIT_ROCKETS; ++i)
  {
    addRocket();
  }
  float time = glfwGetTime();
  float previousRocketTime = 0;
  while(RUNNING)
  {
    float oldTime = time;
    time = glfwGetTime();
    float delta = time - oldTime;

    // INPUT
    glfwPollEvents();

    // UPDATE
    if(time - previousRocketTime > ROCKET_INTERVAL)
    {
      addRocket();
      previousRocketTime = time;
    }

    gasManagerAnimate(manager, delta);

    // RENDER
    glhckRenderClear(GLHCK_DEPTH_BUFFER_BIT | GLHCK_COLOR_BUFFER_BIT);

    for(i = 0; i < NUM_PARTICLES; ++i)
    {
      if(particles[i].alive)
      {
        glhckObjectDraw(particles[i].object);
      }
    }
    glhckRender();

    glfwSwapBuffers(window);
  }

  gasManagerFree(manager);

  glhckContextTerminate();
  glfwTerminate();
  return EXIT_SUCCESS;
}
