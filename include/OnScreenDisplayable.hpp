#pragma once
#include <cstddef>

class OnScreenDisplayable {
public:
  virtual ~OnScreenDisplayable() {}

  virtual void display() = 0;

  virtual void reshape(size_t width, size_t height) = 0;
};
