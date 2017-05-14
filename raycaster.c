#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include <SDL_image.h>
#include <SDL.h>

int BLOCKSIZE = 64;

typedef struct IntMatrix2D {
  int width;
  int height;
  int** matrix;
} IntMatrix2D;

typedef struct FloatMatrix2D {
  int width;
  int height;
  float** matrix;
} FloatMatrix2D;

typedef struct Camera {
  float pos[2];
  float dir[2];
  float plane[2];
} Camera;

typedef struct Player {
  Camera camera;
} Player;

typedef struct ColorRGB {
  float r;
  float g;
  float b;
} ColorRGB;

IntMatrix2D makeMap(char fileStr[]){
  FILE *mapfile = fopen(fileStr, "r");
  IntMatrix2D map;
  char line[5];
  int i, row;
  if (mapfile == NULL) {
    fprintf(stderr, "Unable to open map file.\n");
  }
  fseek(mapfile, 0, SEEK_SET);
  fgets(line, 5, mapfile);
  map.width = (int)strtol(line, (char **)NULL, 10);
  fgets(line, 5, mapfile);
  map.height = (int)strtol(line, (char **)NULL, 10);
  
  map.matrix = (int **) malloc(map.height*sizeof(int));
  for(i = 0; i < map.height; i++){
    map.matrix[i] = (int *) malloc(map.width*sizeof(int));
  }
  if(NULL == map.matrix){
    fprintf(stderr, "malloc failed\n");
  }
  char maprow[map.width];
  for(row = 0; row < map.height; row++){
    fgets(maprow, map.width+2, mapfile); // width + 2 to eat newline
    for(i = 0; i < map.width; i++){
      map.matrix[row][i] = maprow[i] - '0';
    }
  }
  return map;
}

int init_FloatMatrix2D(FloatMatrix2D* m){
  int i;
  m->matrix = (float **) malloc(m->height*sizeof(float));
  for(i = 0; i < m->height; i++){
    m->matrix[i] = (float *) malloc(m->width*sizeof(float));
  }
  if(NULL == m->matrix){
    fprintf(stderr, "malloc failed\n");
    return -1;
  }
  return 0;
}

FloatMatrix2D mmult(FloatMatrix2D A, FloatMatrix2D B){
  FloatMatrix2D result;
  if (A.height == B.width) {
    int i, j, k;
    result.height = A.width;
    result.width = B.height;
    init_FloatMatrix2D(&result);
    for(i = 0; i < A.width - 1; i++){
      for(j = 0; j < B.height - 1; j++){
        float sum = 0.0f;
        for(k = 0; k < A.height -1; k++){
          sum += A.matrix[i][k]*B.matrix[k][j];
        }
        result.matrix[i][j] = sum;
      }
    }
  }
  return result;
}

FloatMatrix2D smult(FloatMatrix2D A, float s){
  FloatMatrix2D result;
  int i, j;
  result.height = A.height;
  result.width = A.width;
  init_FloatMatrix2D(&result);
  for(i = 0; i < A.width - 1; i++){
    for(j = 0; j < A.height - 1; j++){
      result.matrix[i][j] = A.matrix[i][j]*s;
    }
  }
  return result;
}

FloatMatrix2D add(FloatMatrix2D A, FloatMatrix2D B){
  FloatMatrix2D result;
  if((A.width == B.width) && (A.height == B.height)){
    int i, j;
    result.height = A.height;
    result.width = A.width;
    init_FloatMatrix2D(&result);
    for(i = 0; i < A.width - 1; i++){
      for(j = 0; j < A.height - 1; j++){
        result.matrix[i][j] = A.matrix[i][j] + B.matrix[i][j];
      }
    }
  }
  return result;
}

void error_callback(int error, const char* description)
{
  fprintf(stderr, "%s\n", description);
}

// Left, Right, Forward, Backward
int keys[] = {0, 0, 0, 0};

void key_callback(GLFWwindow* window, int key,int scancode, int action, int mods){
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
  if(key == GLFW_KEY_LEFT && action == GLFW_PRESS){
    keys[0] = 1;
  }
  if(key == GLFW_KEY_LEFT && action == GLFW_RELEASE){
    keys[0] = 0;
  }
  if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
    keys[1] = 1;
  }
  if(key == GLFW_KEY_RIGHT && action == GLFW_RELEASE){
    keys[1] = 0;
  }
  if(key == GLFW_KEY_UP && action == GLFW_PRESS){
    keys[2] = 1;
  }
  if(key == GLFW_KEY_UP && action == GLFW_RELEASE){
    keys[2] = 0;
  }
  if(key == GLFW_KEY_DOWN && action == GLFW_PRESS){
    keys[3] = 1;
  }
  if(key == GLFW_KEY_DOWN && action == GLFW_RELEASE){
    keys[3] = 0;
  }
}

void drawLine(float x, float startY, float endY, ColorRGB color){
  glColor3f(color.r, color.g, color.b);
  glBegin(GL_LINES);
  glVertex3f(x, startY, 0.0f);
  glVertex3f(x, endY, 0.0f);
  glEnd();
}

void updatePlayer(Player* player, IntMatrix2D map, float frameTime){
  //fprintf(stdout, "%d, %d : ", player->camera.dir[0], player->camera.dir[1]);
  float rotSpeed = frameTime*5.0;
  float moveSpeed = frameTime*10.0;
  if(keys[0] == 1){
    float oldDirX = player->camera.dir[0];
    player->camera.dir[0] = player->camera.dir[0]*cos(-rotSpeed)-player->camera.dir[1]*sin(-rotSpeed);
    player->camera.dir[1] = oldDirX*sin(-rotSpeed)+player->camera.dir[1]*cos(-rotSpeed);
    float oldPlaneX = player->camera.plane[0];
    player->camera.plane[0] = player->camera.plane[0]*cos(-rotSpeed)-player->camera.plane[1]*sin(-rotSpeed);
    player->camera.plane[1] = oldPlaneX*sin(-rotSpeed)+player->camera.plane[1]*cos(-rotSpeed);
  }
  if(keys[1] == 1){
    float oldDirX = player->camera.dir[0];
    player->camera.dir[0] = player->camera.dir[0]*cos(rotSpeed)-player->camera.dir[1]*sin(rotSpeed);
    player->camera.dir[1] = oldDirX*sin(rotSpeed)+player->camera.dir[1]*cos(rotSpeed);
    float oldPlaneX = player->camera.plane[0];
    player->camera.plane[0] = player->camera.plane[0]*cos(rotSpeed)-player->camera.plane[1]*sin(rotSpeed);
    player->camera.plane[1] = oldPlaneX*sin(rotSpeed)+player->camera.plane[1]*cos(rotSpeed);
  }
  if(keys[2] == 1){
    if(!map.matrix[(int)(player->camera.pos[0] + player->camera.dir[0] * moveSpeed)][(int)player->camera.pos[1]]){
      player->camera.pos[0] += player->camera.dir[0] * moveSpeed;
    }
    if(!map.matrix[(int)(player->camera.pos[0])][(int)(player->camera.pos[1] + player->camera.dir[1] * moveSpeed)]){
      player->camera.pos[1] += player->camera.dir[1] * moveSpeed;
    }
  }
  if(keys[3] == 1){
    if(!map.matrix[(int)(player->camera.pos[0] - player->camera.dir[0] * moveSpeed)][(int)player->camera.pos[1]]){
      player->camera.pos[0] -= player->camera.dir[0] * moveSpeed;
    }
    if(!map.matrix[(int)(player->camera.pos[0])][(int)(player->camera.pos[1] - player->camera.dir[1] * moveSpeed)]){
      player->camera.pos[1] -= player->camera.dir[1] * moveSpeed;
    }
  }
  //fprintf(stdout, "%d, %d\n", player->camera.dir[0], player->camera.dir[1]);
}

static GLuint textures[2];

void draw_floor(IntMatrix2D map){
  int x_step = 1;
  int y_step = 1;
  int x, y;
  glBindTexture(GL_TEXTURE_2D, textures[1]);
  glEnable(GL_TEXTURE_2D);

  for(x = 0; x < map.width; x += x_step){
    for(y = 0; y < map.height; y+=y_step){
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(x, 0.0f, y);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(x+x_step, 0.0f, y);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(x+x_step, 0.0f, y+y_step);

      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(x, 0.0f, y);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(x+x_step, 0.0f, y+y_step);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(x, 0.0f, y+y_step);
      glEnd();
    }
  }
}

static void load_texture(unsigned int idx, const char *filename)
{
  SDL_Surface* surface;

  surface = IMG_Load(filename);
  if (!surface) {
    fprintf(stderr, "%s: error loading texture: %s\n",
            filename, SDL_GetError());
    return;
  }
  glGenTextures(1, &textures[idx]);
  glBindTexture(GL_TEXTURE_2D, textures[idx]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
  SDL_FreeSurface(surface);
}

int* coordToGrid(int coord[2]){
  static int result[2];
  result[0] = coord[0]/BLOCKSIZE;
  result[1] = coord[1]/BLOCKSIZE;
  return result;
}

int main(int argc, const char* argv[])
{
  IntMatrix2D map = makeMap("map.txt");
  glfwSetErrorCallback(error_callback);
  if(!glfwInit()) {
    exit(EXIT_FAILURE);
  } else {
    load_texture(0, "Assets/Textures/Wall.jpg");
    load_texture(1, "Assets/Textures/Floor.jpg");

    Player player;
    Camera camera;
    camera.pos[0] = 4.0f;
    camera.pos[1] = 4.0f;
    camera.dir[0] = -1.0f;
    camera.dir[1] = 0.0f;
    camera.plane[0] = 0.0f;
    camera.plane[1] = 0.66f;
    player.camera = camera; 

    GLFWwindow* window = glfwCreateWindow(512, 384, "Raycaster", NULL, NULL);
    if(!window){
      glfwTerminate();
      fprintf(stderr, "Window creation failed.\n");
      exit(EXIT_FAILURE);
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);
    
    while(!glfwWindowShouldClose(window)){
      int width, height;
      float ratio;
      glfwGetFramebufferSize(window, &width, &height);
      ratio = width / (float) height;
      
      glViewport(0, 0, width, height);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      //glOrtho(-ratio, ratio, -1.0f, 1.0f, 1.0f, -1.0f);
      glOrtho(0.0f, (float) width, (float) height, 0.0f, 1.0f, -1.0f);
      glMatrixMode(GL_MODELVIEW);
      draw_floor(map);
      //glLoadIdentity();

      float time = glfwGetTime();
      float oldTime = glfwGetTime();
      float cameraX = 0;
      float rayDir[2];
      int x, hit, side;
      int mapVec[2], step[2];
      float sideDist[2], deltaDist[2], rayPos[2];
      float perpWallDist;
      for(x = 0; x < width; x++){
        cameraX = 2 * x / (float)width - 1;
        rayPos[0] = player.camera.pos[0];
        rayPos[1] = player.camera.pos[1];
        rayDir[0] = player.camera.dir[0] + cameraX*player.camera.plane[0];
        rayDir[1] = player.camera.dir[1] + cameraX*player.camera.plane[1];
        mapVec[0] = (int)rayPos[0];
        mapVec[1] = (int)rayPos[1];
        deltaDist[0] = sqrt(1 + (rayDir[1]*rayDir[1]) / (rayDir[0]*rayDir[0]));
        deltaDist[1] = sqrt(1 + (rayDir[0]*rayDir[0]) / (rayDir[1]*rayDir[1]));
        hit = 0;
        
        if(rayDir[0] < 0){
          step[0] = -1;
          sideDist[0] = (rayPos[0] - mapVec[0])*deltaDist[0];
        } else {
          step[0] = 1;
          sideDist[0] = (mapVec[0] + 1.0 - rayPos[0])*deltaDist[0];
        }
        if(rayDir[1] < 0){
          step[1] = -1;
          sideDist[1] = (rayPos[1] - mapVec[1])*deltaDist[1];
        } else {
          step[1] = 1;
          sideDist[1] = (mapVec[1] + 1.0 - rayPos[1])*deltaDist[1];
        }

        while(hit == 0){
          if(sideDist[0] < sideDist[1]){
            sideDist[0] += deltaDist[0];
            mapVec[0] += step[0];
            side = 0;
          } else {
            sideDist[1] += deltaDist[1];
            mapVec[1] += step[1];
            side = 1;
          }
          if (map.matrix[mapVec[0]][mapVec[1]] > 0){
            hit = 1;
          }
        }
        
        if(side == 0){
          perpWallDist = (mapVec[0] - rayPos[0] + (1.0 - step[0]) / 2.0f) / rayDir[0];
        } else {
          perpWallDist = (mapVec[1] - rayPos[1] + (1.0 - step[1]) / 2.0f) / rayDir[1];
        }
        int lineHeight = (int)(height / (float)perpWallDist);
        int drawStart = -lineHeight / 2.0f + height / 2.0f;
        if(drawStart < 0){
          drawStart = 0;
        }
        int drawEnd = lineHeight / 2.0f + height / 2.0f;
        if(drawEnd >= height){
          drawEnd = height - 1;
        }
        ColorRGB color;
        switch(map.matrix[mapVec[0]][mapVec[1]]){
        case 1:
          color = (ColorRGB) {1.0f, 0.0f, 0.0f};
          break;
        case 2:
          color = (ColorRGB) {0.0f, 1.0f, 0.0f};
          break;
        case 3:
          color = (ColorRGB) {0.0f, 0.0f, 1.0f};
          break;
        case 4:
          color = (ColorRGB) {1.0f, 1.0f, 1.0f};
          break;
        default:
          color = (ColorRGB) {0.0f, 1.0f, 1.0f};
          break;
        }
        if(side == 1){
          color.r *= 0.5f;
          color.g *= 0.5f;
          color.b *= 0.5f;
        }
        drawLine(x, drawStart, drawEnd, color);
      }
      oldTime = time;
      time = glfwGetTime();
      float frameTime = (time - oldTime);
      updatePlayer(&player, map, frameTime);
      glfwSwapBuffers(window);
      glfwPollEvents();
      glFlush();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
  }
  exit(EXIT_SUCCESS);
}
