#pragma once
#include "ShaderProgram.hpp"
#include "Texture.hpp"
#include <cstddef>

class TextureDisplay {
  float scale;
  ShaderProgram displayShaderProgram;
  GLuint vao;
  GLuint vbo;

public:
  size_t screenWidth;
  size_t screenHeight;

  TextureDisplay(size_t screenWidth, size_t screenHeight, float scale = 1.0f)
      : screenWidth(screenWidth), screenHeight(screenHeight), scale(scale),
        displayShaderProgram("display shader") {
    displayShaderProgram.attachShader(
        Shader("vertex", "shader/displayVs.glsl", ShaderType::VERTEX));
    displayShaderProgram.attachShader(
        Shader("fragment", "shader/displayFs.glsl", ShaderType::FRAGMENT));
    displayShaderProgram.link();
    displayShaderProgram.bind();

    /********** setup the drawing primitive **********/
    static const GLfloat vertex_positions[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), vertex_positions, GL_STATIC_DRAW);
    displayShaderProgram.vertexAttribPointer("pos", 2, GL_FLOAT, 0, 0, false);
    glEnableVertexAttribArray(displayShaderProgram.attributeLocation("pos"));
  }

  ~TextureDisplay() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
  }

  void reshape(size_t screenWidth, size_t screenHeight) {
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
  }

  void display(const Texture &texture, size_t posX, size_t posY) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    displayShaderProgram.bind();
    displayShaderProgram.setUniform2f("size", scale * (GLfloat)texture.width / screenWidth,
                                      scale * (GLfloat)texture.height / screenHeight);
    displayShaderProgram.setUniform2f("position",
                                      (2.0f * posX - screenWidth + texture.width) / screenWidth,
                                      (2.0f * posY - screenHeight + texture.height) / screenHeight);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
};
