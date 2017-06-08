#include "StatusBar.hpp"
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

StatusBar::StatusBar(size_t screenWidth, size_t screenHeight, const std::string &fontFilename,
                     size_t fontSize, SDL_Color fontColor, SDL_Color outlineFontColor)
    : statFont(TTF_OpenFont(fontFilename.c_str(), fontSize)), fontColor(fontColor),
      outlineFontColor(outlineFontColor), textureDisplay(screenWidth, screenHeight) {
  reshape(screenWidth, screenHeight);
  if (statFont == nullptr) {
    std::cerr << "Error while opening font" << std::endl;
    exit(EXIT_FAILURE);
  }
  refreshStatusBar();
}

StatusBar::~StatusBar() { TTF_CloseFont(statFont); }

void StatusBar::refreshStatusBar() {
  std::ostringstream statString;
  statString << "FPS: " << std::fixed << std::setw(9) << std::setprecision(3)
             << 1.0 / (std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0) /
                       elapsedTimes.size())
             << ", SPP: " << std::setw(4) << sampleCount;
  auto text = statString.str();
  TTF_SetFontOutline(statFont, 0);
  SDL_Surface *statSurface = TTF_RenderText_Blended(statFont, text.c_str(), fontColor);
  TTF_SetFontOutline(statFont, 1);
  SDL_Surface *statSurfaceOutl = TTF_RenderText_Blended(statFont, text.c_str(), outlineFontColor);
  SDL_Rect dstClip;
  dstClip.x = 1;
  dstClip.y = 1;
  dstClip.w = statSurface->w;
  dstClip.h = statSurface->h;
  SDL_BlitSurface(statSurface, nullptr, statSurfaceOutl, &dstClip);
  statSurfaceTexture.createTextureFromPixelData(statSurfaceOutl->w, statSurfaceOutl->h,
                                                statSurfaceOutl->pixels);
  SDL_FreeSurface(statSurface);
  SDL_FreeSurface(statSurfaceOutl);
}

void StatusBar::setDeltaTimeStep(double elapsedTime) {
  elapsedTimes.push_back(elapsedTime);
  while (elapsedTimes.size() > 10)
    elapsedTimes.pop_front();
  refreshStatusBar();
}

void StatusBar::setSampleCount(size_t sampleCount) {
  this->sampleCount = sampleCount;
  refreshStatusBar();
}

void StatusBar::display() {
  textureDisplay.display(statSurfaceTexture, 0,
                         textureDisplay.screenHeight - statSurfaceTexture.height);
}

void StatusBar::reshape(size_t screenWidth, size_t screenHeight) {
  textureDisplay.reshape(screenWidth, screenHeight);
}
