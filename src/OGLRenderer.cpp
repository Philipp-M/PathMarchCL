#include "OGLRenderer.hpp"
#include <iostream>
#include <sstream>

OGLRenderer::OGLRenderer(size_t width, size_t height) : needUpdate(true) {
  GLenum rev;
  glewExperimental = GL_TRUE;
  rev = glewInit();

  if (GLEW_OK != rev) {
    std::cerr << "Error: " << glewGetErrorString(rev) << std::endl;
    exit(1);
  }

  /********** OpenCL initialization **********/
  oclRenderer.reset(new OCLRenderer(width, height, "raymarch", "kernels/kernels.cl"));

  /********** setup shader **********/
  renderShaderProgram.reset(new ShaderProgram("render"));
  renderShaderProgram->attachShader(Shader("vertex", "shader/renderVs.glsl", ShaderType::VERTEX));
  renderShaderProgram->attachShader(
      Shader("fragment", "shader/renderFs.glsl", ShaderType::FRAGMENT));
  renderShaderProgram->link();
  renderShaderProgram->bind();

  /********** setup other raytracing/raymarching specific stuff *********/
  needUpdate = true;

  /********** setup the drawing primitive **********/
  static const GLfloat vertex_positions[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

  glGenVertexArrays(1, &renderVao);
  glBindVertexArray(renderVao);
  glGenBuffers(1, &renderVbo);
  glBindBuffer(GL_ARRAY_BUFFER, renderVbo);
  glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), vertex_positions, GL_STATIC_DRAW);
  renderShaderProgram->vertexAttribPointer("pos", 2, GL_FLOAT, 0, 0, false);
  glEnableVertexAttribArray(renderShaderProgram->attributeLocation("pos"));

  reshape(width, height);
}

OGLRenderer::~OGLRenderer() {
  glDeleteBuffers(1, &renderVbo);
  glDeleteVertexArrays(1, &renderVao);
}

void OGLRenderer::reshape(int width, int height) {
  glViewport(0, 0, width, height);
  if (oclRenderer != nullptr)
    oclRenderer->reshape(width, height);
  refresh();
  display();
}

void OGLRenderer::display() {
  oclRenderer->render(needUpdate);
  needUpdate = false;
  renderShaderProgram->bind();
  renderShaderProgram->setUniform1i("srcTex", 0);
  glBindTexture(GL_TEXTURE_2D, oclRenderer->getTexture().id);
  glBindBuffer(GL_ARRAY_BUFFER, renderVbo);
  glBindVertexArray(renderVao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OGLRenderer::refresh() { needUpdate = true; }

void OGLRenderer::setVMatrix(glm::mat4 m) {
  oclRenderer->setVMatrix(m);
  refresh();
}

void OGLRenderer::setFov(float fov) {
  oclRenderer->setFov(fov);
  refresh();
}

size_t OGLRenderer::getSampleCount() { return oclRenderer->getSampleCount(); }

void OGLRenderer::saveRenderedImage(const std::string &filenamePrefix) {
  glFinish();
  size_t width = oclRenderer->getTexture().width;
  size_t height = oclRenderer->getTexture().height;
  uint32_t *pixels = new uint32_t[width * height];
  auto rawImage = oclRenderer->getImage();

  for (size_t i = 0; i < width * height * 4; i += 4)
    pixels[i / 4] = (((uint32_t)rawImage[i + 0]) << 24) | (((uint32_t)rawImage[i + 1]) << 16) |
                    (((uint32_t)rawImage[i + 2]) << 8) | (((uint32_t)rawImage[i + 3]) << 0);

  SDL_Surface *image = SDL_CreateRGBSurfaceFrom(pixels, width, height, 24, 4 * width, 0xFF000000,
                                                0x00FF0000, 0x0000FF00, 0x000000FF);
  std::ostringstream filename;
  filename << filenamePrefix << time(nullptr) << "_" << oclRenderer->getSampleCount() << "SPP.bmp";
  SDL_SaveBMP(image, filename.str().c_str());
  SDL_FreeSurface(image);
  delete[] pixels;
}
