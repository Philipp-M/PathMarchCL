#pragma once
#include "OnScreenDisplayable.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"
#include "TextureDisplay.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <deque>
#include <string>

class StatusBar : public OnScreenDisplayable {
  TTF_Font *statFont;
  SDL_Color fontColor;
  SDL_Color outlineFontColor;
  Texture statSurfaceTexture;
  TextureDisplay textureDisplay;
  std::deque<double> elapsedTimes;
  size_t sampleCount;
  std::shared_ptr<ShaderProgram> displayShader;

  void refreshStatusBar();

public:
  StatusBar(size_t screenWidth, size_t screenHeight, const std::string &fontFilename,
             size_t fontSize, SDL_Color fontColor = {255, 255, 255, 255},
             SDL_Color outlineFontColor = {0, 0, 0, 255});

  ~StatusBar();

  /**
   * set the time in seconds
   */
  void setDeltaTimeStep(double time);

  void setSampleCount(size_t sampleCount);

  void display();

  void reshape(size_t screenWidth, size_t screenHeight);
};
