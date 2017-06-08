#include "App.hpp"
#include "common.hpp"

App::App(SDL_Window *window)
    : window(window), movementSpeed(2.0f), fov(2.0f), camera(glm::vec3(0.0f, 0.0f, -1.0f)),
      quit(false) {
  deltaElapsedTime = 0;
  curTime = Clock::now();

  /********** Other GL related stuff **********/
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);

  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  oglRenderer.reset(new OGLRenderer(w, h));
  oglRenderer->setVMatrix(camera.getViewMatrix());
  oglRenderer->setFov(fov);

  /********** initialize the on screen displayables **********/
  statusBar = std::make_shared<StatusBar>(w, h, "assets/Roboto-Regular.ttf", 20);
  onScreenDisplayables.push_back(statusBar);
}

App::~App() {}

void App::mainLoop() {
  while (!quit) {
    deltaElapsedTime = (double)getPastTime(curTime) / 1.0e9;
    curTime = Clock::now();
    statusBar->setDeltaTimeStep(deltaElapsedTime);
    statusBar->setSampleCount(oglRenderer->getSampleCount());
    processEvents();
    processKeys();
    display();
  }
}

void App::processEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      quit = true;
      break;
    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_RESIZED)
        reshape(event.window.data1, event.window.data2);
      break;
    case SDL_MOUSEMOTION:
      mouseMovement(event.motion.xrel, event.motion.yrel);
      break;
    case SDL_MOUSEWHEEL:
      mouseWheel(event.wheel.y);
      break;
    case SDL_KEYDOWN:
      if (event.key.repeat == 0)
        pressedKeys[event.key.keysym.sym] = true;
      break;
    case SDL_KEYUP:
      if (event.key.repeat == 0)
        pressedKeys[event.key.keysym.sym] = false;
      break;
    }
  }
}

void App::processKeys() {
  bool needUpdate = false;
  if (pressedKeys[SDLK_w] || pressedKeys[SDLK_UP]) {
    camera.advance(-movementSpeed * deltaElapsedTime);
    needUpdate = true;
  }
  if (pressedKeys[SDLK_a] || pressedKeys[SDLK_LEFT]) {
    camera.strafe(-movementSpeed * deltaElapsedTime);
    needUpdate = true;
  }
  if (pressedKeys[SDLK_s] || pressedKeys[SDLK_DOWN]) {
    camera.advance(movementSpeed * deltaElapsedTime);
    needUpdate = true;
  }
  if (pressedKeys[SDLK_d] || pressedKeys[SDLK_RIGHT]) {
    camera.strafe(movementSpeed * deltaElapsedTime);
    needUpdate = true;
  }
  if (pressedKeys[SDLK_e]) {
    camera.roll(-deltaElapsedTime);
    needUpdate = true;
  }
  if (pressedKeys[SDLK_q]) {
    camera.roll(deltaElapsedTime);
    needUpdate = true;
  }
  if (pressedKeys[SDLK_f] && !oldPressedKeys[SDLK_f]) {
    camera.setFps(!camera.getFps());
    needUpdate = true;
  }
  if ((pressedKeys[SDLK_PLUS] && !oldPressedKeys[SDLK_PLUS]) ||
      (pressedKeys[SDLK_EQUALS] && !oldPressedKeys[SDLK_EQUALS]))
    movementSpeed *= 1.25;
  if (pressedKeys[SDLK_MINUS] && !oldPressedKeys[SDLK_MINUS])
    movementSpeed /= 1.25;

  if (pressedKeys[SDLK_b] && !oldPressedKeys[SDLK_b])
    oglRenderer->setFov(fov *= 1.1f);
  if (pressedKeys[SDLK_v] && !oldPressedKeys[SDLK_v])
    oglRenderer->setFov(fov /= 1.1f);

  if (pressedKeys[SDLK_i] && !oldPressedKeys[SDLK_i])
    oglRenderer->saveRenderedImage("render_");

  if (pressedKeys[SDLK_x])
    quit = true;
  oldPressedKeys = pressedKeys;

  // set the new view matrix if it has changed
  if (needUpdate)
    oglRenderer->setVMatrix(camera.getViewMatrix());
}

void App::mouseMovement(int deltaX, int deltaY) {
  if (deltaX < 100)
    camera.yaw(-0.001f * deltaX);
  if (deltaY < 100)
    camera.pitch(-0.001f * deltaY);
  oglRenderer->setVMatrix(camera.getViewMatrix());
}

void App::mouseWheel(int deltaY) { movementSpeed *= deltaY > 0 ? 1.25f : 0.8f; }

void App::display() {
  glClear(GL_COLOR_BUFFER_BIT);
  oglRenderer->display();
  for (auto &osd : onScreenDisplayables)
    osd->display();
  SDL_GL_SwapWindow(window);
}

void App::reshape(size_t width, size_t height) {
  oglRenderer->reshape(width, height);
  for (auto &osd : onScreenDisplayables) {
    osd->reshape(width, height);
  }
}
