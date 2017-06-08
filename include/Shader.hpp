#pragma once

#include <GL/glew.h>
#include <string>

enum ShaderType { VERTEX, FRAGMENT, GEOMETRY, COMPUTE };

class Shader {
  GLuint shaderId;
  std::string name;
  ShaderType type;

  GLuint load(const std::string &filename);

public:
  Shader(const std::string &name, const std::string &filename, ShaderType type);

  ShaderType getType() const;

  GLuint getShaderId() const;

  const std::string &getName() const;

  void setName(std::string &name);
};
