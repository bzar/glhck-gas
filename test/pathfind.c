#include "GLFW/glfw3.h"
#include "glhck/glhck.h"
#include "gas.h"

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 800
#define HEIGHT 480

#define LEVEL_SIZE 8
#define GRID_SIZE 8

int LEVEL[LEVEL_SIZE * LEVEL_SIZE] = {
  1,2,2,2,3,3,4,4,
  1,1,2,2,3,2,3,4,
  1,1,2,2,3,3,3,4,
  1,1,2,2,3,3,4,5,
  3,4,4,5,3,4,5,6,
  3,4,5,5,3,4,5,7,
  8,5,6,5,4,5,6,6,
  9,8,9,7,6,7,6,9,
};

char RUNNING = 1;
double CURSOR_X = 0;
double CURSOR_Y = 0;
char MOUSE_BUTTON_1;

typedef struct LevelPosition {
  int x;
  int z;
} LevelPosition;

void windowCloseCallback(GLFWwindow* window)
{
  RUNNING = 0;
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
  CURSOR_X = x;
  CURSOR_Y = y;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  if(button == GLFW_MOUSE_BUTTON_LEFT)
  {
    MOUSE_BUTTON_1 = action == GLFW_PRESS;
  }
}

glhckObject** createLevelObjects(int* level, int levelSize)
{
  glhckObject** objects = calloc(levelSize * levelSize, sizeof(glhckObject*));

  int i;
  for(i = 0; i < levelSize * levelSize; ++i)
  {
    int x = i % levelSize;
    int y = level[i];
    int z = i / levelSize;

    glhckObject* object = glhckCubeNew(GRID_SIZE/2);
    glhckObjectScalef(object, GRID_SIZE/2, y*GRID_SIZE/2, GRID_SIZE/2);
    glhckObjectPositionf(object, x * GRID_SIZE, y/2.0f * GRID_SIZE, z * GRID_SIZE);
    glhckObjectMaterial(object, glhckMaterialNew(NULL));
    glhckMaterialDiffuseb(glhckObjectGetMaterial(object),
                          (y * 16) + (x + z) % 2 * 32 + x * 8,
                          (y * 16) + (x + z) % 2 * 32 + z * 8,
                          (y * 16) + (x + z) % 2 * 32 + (x + z) * 4,
                          255);
    objects[i] = object;
  }

  return objects;
}

kmRay3* createPointerRay(kmRay3* pOut, float const x, float const y, float const width, float const height, glhckFrustum const* frustum)
{
  kmVec3 nu, nv, fu, fv;
  kmVec3Subtract(&nu, &frustum->nearCorners[GLHCK_FRUSTUM_CORNER_BOTTOM_RIGHT], &frustum->nearCorners[GLHCK_FRUSTUM_CORNER_BOTTOM_LEFT]);
  kmVec3Subtract(&nv, &frustum->nearCorners[GLHCK_FRUSTUM_CORNER_TOP_LEFT], &frustum->nearCorners[GLHCK_FRUSTUM_CORNER_BOTTOM_LEFT]);
  kmVec3Subtract(&fu, &frustum->farCorners[GLHCK_FRUSTUM_CORNER_BOTTOM_RIGHT], &frustum->farCorners[GLHCK_FRUSTUM_CORNER_BOTTOM_LEFT]);
  kmVec3Subtract(&fv, &frustum->farCorners[GLHCK_FRUSTUM_CORNER_TOP_LEFT], &frustum->farCorners[GLHCK_FRUSTUM_CORNER_BOTTOM_LEFT]);
  kmVec2 relativeMousePos = { x/width, (height-y)/height };

  kmVec3Scale(&nu, &nu, relativeMousePos.x);
  kmVec3Scale(&nv, &nv, relativeMousePos.y);
  kmVec3Scale(&fu, &fu, relativeMousePos.x);
  kmVec3Scale(&fv, &fv, relativeMousePos.y);

  pOut->start = frustum->nearCorners[GLHCK_FRUSTUM_CORNER_BOTTOM_LEFT];
  pOut->dir = frustum->farCorners[GLHCK_FRUSTUM_CORNER_BOTTOM_LEFT];

  kmVec3Add(&pOut->start, &pOut->start, &nu);
  kmVec3Add(&pOut->start, &pOut->start, &nv);
  kmVec3Add(&pOut->dir, &pOut->dir, &fu);
  kmVec3Add(&pOut->dir, &pOut->dir, &fv);
  kmVec3Subtract(&pOut->dir, &pOut->dir, &pOut->start);

  return pOut;
}

typedef struct PathNode {
  int x;
  int z;
  int cost;
  struct PathNode* from;
  struct PathNode* next;
} PathNode;

int nodeDistance(PathNode* node, int x, int z)
{
  return abs(node->x - x) + abs(node->z - z);
}

LevelPosition* findPath(int fromX, int fromZ, int toX, int toZ, int* level, int levelSize, int* pathLength)
{
  PathNode* queue = calloc(1, sizeof(PathNode));
  queue->x = fromX;
  queue->z = fromZ;
  queue->cost = 0;
  queue->from = NULL;
  queue->next = NULL;

  PathNode* visited = NULL;
  PathNode* result = NULL;

  while (queue)
  {
    // Take node from queue
    PathNode* node = queue;

    // Check end condition
    if(node->x == toX && node->z == toZ)
    {
      result = node;
      break;
    }

    // Remove node from queue
    queue = node->next;

    // Find previous route to same location
    PathNode* existing;
    for(existing = visited; existing && (existing->x != node->x || existing->z != node->z); existing = existing->next);

    // ...if found, check if it was shorter. If it was, discard this route, else replace that route with this.
    if(existing)
    {
      if(existing->cost <= node->cost)
      {
        free(node);
        continue;
      }
      else
      {
        existing->cost = node->cost;
        existing->from = node->from;
        free(node);
        node = existing;
      }
    }

    // Define neighbor nodes
    PathNode neighbors[4] = {
      { node->x - 1, node->z, node->cost + 1, node, NULL },
      { node->x + 1, node->z, node->cost + 1, node, NULL },
      { node->x, node->z - 1, node->cost + 1, node, NULL },
      { node->x, node->z + 1, node->cost + 1, node, NULL }
    };

    int i;
    for (i = 0; i < 4; ++i)
    {
      // Check if neighbor is inside the level
      if (neighbors[i].x < 0 || neighbors[i].x >= levelSize ||
         neighbors[i].z < 0 || neighbors[i].z >= levelSize)
        continue;

      // Check if neighbor can be moved to
      if (level[neighbors[i].x + neighbors[i].z * levelSize] - level[node->x + node->z * levelSize] > 2)
        continue;

      // Create node for neighbor
      PathNode* child = calloc(1, sizeof(PathNode));
      *child = neighbors[i];

      // Insert child node to queue
      if (queue && nodeDistance(queue, toX, toZ) <= nodeDistance(child, toX, toZ))
      {
        PathNode* n;
        for(n = queue; n->next && nodeDistance(n->next, toX, toZ) <= nodeDistance(child, toX, toZ); n = n->next);
        child->next = n->next;
        n->next = child;
      }
      else
      {
        child->next = queue;
        queue = child;
      }
    }

    // Insert node to visited if not already there
    if(!existing)
    {
      node->next = visited;
      visited = node;
    }
  }

  *pathLength = 0;
  LevelPosition* path = NULL;

  if (result)
  {
    PathNode* node;
    // Determine path length
    for(node = result; node; node = node->from)
      *pathLength += 1;

    // Populate path array
    path = calloc(*pathLength, sizeof(LevelPosition));
    int i;
    for(i = *pathLength - 1, node = result; node; --i, node = node->from)
    {
      path[i].x = node->x;
      path[i].z = node->z;
    }
  }

  {
    // Free nodes
    PathNode* node;
    for(node = queue; node; node = node->next)
      free(node);
    for(node = visited; node; node = node->next)
      free(node);
  }

  return path;
}

gasAnimation* moveStep(kmVec3 const* from, kmVec3 const* to)
{
  gasAnimation* x = gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_X, GAS_EASING_LINEAR, from->x, to->x, 1.0f);
  float peak = (from->y > to->y ? from->y : to->y) + 4.0f;
  float t = (from->y > to->y ? 0.3f : from->y < to->y ? 0.7f : 0.5f);
  gasAnimation* ys[2] = {
    gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_Y, GAS_EASING_QUAD_OUT, from->y, peak, t),
    gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_Y, GAS_EASING_QUAD_IN, peak, to->y, 1.0f - t)
  };
  gasAnimation* y = gasSequentialAnimationNew(ys, 2);

  gasAnimation* z = gasNumberAnimationNewFromTo(GAS_NUMBER_ANIMATION_TARGET_Z, GAS_EASING_LINEAR, from->z, to->z, 1.0f);
  gasAnimation* animations[] = {x, y, z};
  return gasParallelAnimationNew(animations, 3);
}


gasAnimation* move(int fromX, int fromZ, int toX, int toZ, int* level, int levelSize)
{
  int pathLength;
  LevelPosition* path = findPath(fromX, fromZ, toX, toZ, level, levelSize, &pathLength);

  if(!path)
  {
    return NULL;
  }

  gasAnimation** animations = calloc(pathLength - 1, sizeof(gasAnimation*));

  int i;
  for(i = 1; i < pathLength; ++i)
  {
    kmVec3 from = { path[i-1].x * GRID_SIZE,
                    (level[path[i-1].x + path[i-1].z * levelSize]) * GRID_SIZE + GRID_SIZE/4,
                    (path[i-1].z) * GRID_SIZE};
    kmVec3 to = { path[i].x * GRID_SIZE,
                  (level[path[i].x + path[i].z * levelSize]) * GRID_SIZE + GRID_SIZE/4,
                  path[i].z * GRID_SIZE};
    animations[i - 1] = moveStep(&from, &to);
  }

  gasAnimation* animation = gasSequentialAnimationNew(animations, pathLength - 1);
  free(animations);
  free(path);

  return animation;
}

int main(int argc, char** argv)
{
  if (!glfwInit())
    return EXIT_FAILURE;

  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "gas-test-pathfind", NULL, NULL);
  glfwMakeContextCurrent(window);

  if(!window)
  {
    return EXIT_FAILURE;
  }

  glfwSetWindowCloseCallback(window, windowCloseCallback);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);

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

  glhckObject** levelObjects = createLevelObjects(LEVEL, LEVEL_SIZE);

  glhckCamera* camera = glhckCameraNew();
  glhckCameraProjection(camera, GLHCK_PROJECTION_ORTHOGRAPHIC);
  glhckObjectPositionf(glhckCameraGetObject(camera), -GRID_SIZE*LEVEL_SIZE, LEVEL_SIZE * GRID_SIZE*2.5, -GRID_SIZE*LEVEL_SIZE);
  glhckObjectTargetf(glhckCameraGetObject(camera), LEVEL_SIZE * GRID_SIZE / 2, LEVEL_SIZE * GRID_SIZE / 2, LEVEL_SIZE * GRID_SIZE / 2);
  glhckCameraRange(camera, 0.1f, 1000.0f);
  glhckCameraFov(camera, 45);
  glhckCameraUpdate(camera);

  int px = 0;
  int pz = 0;
  int py = LEVEL[pz * LEVEL_SIZE + px];
  glhckObject* player = glhckSphereNew(GRID_SIZE / 4);
  glhckObjectPositionf(player, px * GRID_SIZE, py * GRID_SIZE + (GRID_SIZE/4), pz * GRID_SIZE);

  gasAnimation* currentAnimation = NULL;

  glhckText* text = glhckTextNew(256, 256);
  glhckTextColorb(text, 200, 200, 200, 255);
  int fontSize = 12;
  unsigned int font = glhckTextFontNewKakwafont(text, &fontSize);

  char mousePosText[256] = {'\0'};

  int i;
  float time = glfwGetTime();
  while(RUNNING)
  {
    float oldTime = time;
    time = glfwGetTime();
    float delta = time - oldTime;

    // INPUT
    glfwPollEvents();

    glhckFrustum* frustum = glhckCameraGetFrustum(camera);
    kmRay3 pointerRay;
    createPointerRay(&pointerRay, CURSOR_X, CURSOR_Y, WIDTH, HEIGHT, frustum);

    for(i = 0; i < LEVEL_SIZE * LEVEL_SIZE; ++i)
    {
      int x = i % LEVEL_SIZE;
      int y = LEVEL[i];
      int z = i / LEVEL_SIZE;

      kmVec3 topPoint = {x * GRID_SIZE, y * GRID_SIZE, z * GRID_SIZE};
      kmVec3 topNormal = {0, 1, 0};
      kmPlane topPlane;
      kmVec3 intersection, difference;

      kmPlaneFromPointAndNormal(&topPlane, &topPoint, &topNormal);
      kmRay3IntersectPlane(&intersection, &pointerRay, &topPlane);
      kmVec3Subtract(&difference, &topPoint, &intersection);
      float distance = abs(difference.x) > abs(difference.z) ? abs(difference.x) : abs(difference.z);
      glhckObject* object = levelObjects[i];

      if(distance < GRID_SIZE/2)
      {
        glhckObjectDrawOBB(object, 1);

        if(MOUSE_BUTTON_1 && !currentAnimation)
        {
          currentAnimation = move(px, pz, x, z, LEVEL, LEVEL_SIZE);
          if(currentAnimation)
          {
            px = x;
            pz = z;
          }
        }
      }
      else
      {
        glhckObjectDrawOBB(object, 0);
      }
    }

    // UPDATE
    if(currentAnimation)
    {
      if(!gasAnimate(currentAnimation, player, delta))
      {
        gasAnimationFree(currentAnimation);
        currentAnimation = NULL;
      }
    }

    // RENDER
    glhckRenderClear(GLHCK_DEPTH_BUFFER | GLHCK_COLOR_BUFFER);

    for(i = 0; i < LEVEL_SIZE * LEVEL_SIZE; ++i)
    {
      glhckObjectDraw(levelObjects[i]);
    }
    glhckObjectDraw(player);
    glhckCameraUpdate(camera);
    glhckRender();

    glhckTextClear(text);
    sprintf(mousePosText, "Mouse position: (%f, %f) | (%f, %f, %f)->(%f, %f, %f)",
            CURSOR_X, CURSOR_Y,
            pointerRay.start.x, pointerRay.start.y, pointerRay.start.z,
            pointerRay.dir.x, pointerRay.dir.y, pointerRay.dir.z);
    glhckTextStash(text, font, fontSize, 0, fontSize, mousePosText, NULL);
    glhckTextRender(text);

    glfwSwapBuffers(window);
  }

  for(i = 0; i < LEVEL_SIZE * LEVEL_SIZE; ++i)
  {
    glhckObjectFree(levelObjects[i]);
  }
  free(levelObjects);

  glhckObjectFree(player);

  glhckContextTerminate();
  glfwTerminate();
  return EXIT_SUCCESS;
}
