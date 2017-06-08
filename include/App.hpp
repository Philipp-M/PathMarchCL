#pragma once

#include "Camera.hpp"
#include "OGLRenderer.hpp"
#include "OnScreenDisplayable.hpp"
#include "StatusBar.hpp"
#include <SDL2/SDL.h>
#include <chrono>
#include <map>
#include <memory>
#include <vector>

class App {
  SDL_Window *window;
  std::shared_ptr<OGLRenderer> oglRenderer; // the main renderer
  std::vector<std::shared_ptr<OnScreenDisplayable>> onScreenDisplayables;
  std::shared_ptr<StatusBar> statusBar;
  std::map<SDL_Keycode, bool> pressedKeys;
  std::map<SDL_Keycode, bool> oldPressedKeys;
  float movementSpeed; // the speed the camera moves around
  float fov;           // has to be larger than 0, where larger values mean a smaller FOV
  std::chrono::system_clock::time_point curTime;
  double deltaElapsedTime; // elapsed time between each frame in ms
  Camera camera;
  bool quit;

  /**
   * displays the rendered frame inclusive of on screen displays such as FPS
   */
  void display();

  /**
   * resizes the app to the new screen resolution
   */
  void reshape(size_t width, size_t height);

  /**
   * processes events like keystrokes, mouse events or the quit event
   */
  void processEvents();

  /**
   * handles all the keystrokes that are determined in 'processEvents'
   * e.g. if the user presses forward, that the camera moves forward etc.
   */
  void processKeys();

  void mouseMovement(int deltaX, int deltaY);

  void mouseWheel(int deltaY);

public:
  /**
   * the app has to be initialized after SDL 2 is completely intialized, that includes the creation
   * of an opengl context
   */
  App(SDL_Window *window);

  ~App();

  /**
   * the main loop is blocking, handles all the input and draws to the screen
   */
  void mainLoop();
};
