#pragma once

#include "OCLRenderer.hpp"
#include "ShaderProgram.hpp"
#include <SDL2/SDL.h>
#include <chrono>
#include <map>
#include <memory>

class OGLRenderer {
  std::shared_ptr<ShaderProgram> renderShaderProgram;
  std::shared_ptr<OCLRenderer> oclRenderer;
  GLuint renderVao;
  GLuint renderVbo;
  bool needUpdate; // if this is set to true, the samples per pixel will be resetted

public:
  OGLRenderer(size_t width, size_t height);

  ~OGLRenderer();

  /**
   * displays the rendered texture
   */
  void display();

  /**
   * if something happened, like movement this function has to be called
   */
  void refresh();

  /**
   * sets the view matrix for the renderer
   */
  size_t getSampleCount();

  /**
   * resizes the opengl screen and does the same to the opencl renderer
   */
  void reshape(int width, int height);

  /**
   * sets the view matrix for the renderer
   */
  void setVMatrix(glm::mat4 m);

  /**
   * sets the view matrix for the renderer
   */
  void setFov(float fov);

  /**
   * saves a screencapture in the current directory with the following name scheme:
   * {filenamePrefix}{CURRENT_TIME}_{SAMPLE_COUNT}_Spp.bmp
   */
  void saveRenderedImage(const std::string &filenamePrefix = "render_");
};
