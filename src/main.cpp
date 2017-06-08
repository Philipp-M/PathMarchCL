#include "App.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <iostream>

#define PROGRAM_NAME "PathMarchCL"

const size_t WIDTH = 1280;
const size_t HEIGHT = 720;

int main(int argc, char *argv[]) {
  // disable cuda cache because it doesn't recompile the opencl kernels otherwise for some reason
  setenv("CUDA_CACHE_DISABLE", "1", 1);
  SDL_Window *mainwindow;
  SDL_GLContext maincontext;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) { // Initialize SDL's Video subsystem
    std::cerr << "Unable to initialize SDL" << std::endl;
    return EXIT_FAILURE;
  }
  // Request opengl 3.3 context.
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  mainwindow =
      SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                       SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!mainwindow) { // Die if creation failed
    std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // initialize TTF rendering
  if (TTF_Init() == -1) {
    std::cerr << "SDL Error: Failed to initialize TTF : " << SDL_GetError() << std::endl;
  }

  maincontext = SDL_GL_CreateContext(mainwindow);

  // disable vsync
  SDL_GL_SetSwapInterval(0);

  // enable mouse catching
  SDL_SetRelativeMouseMode(SDL_TRUE);

  auto app = std::make_shared<App>(mainwindow);
  // the main loop
  app->mainLoop();

  app.reset();

  SDL_GL_DeleteContext(maincontext);
  SDL_DestroyWindow(mainwindow);
  SDL_Quit();
  return EXIT_SUCCESS;
}
