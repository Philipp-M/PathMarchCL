#pragma once

#include <cmath>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <iostream>

class Camera {
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 forward;
  glm::vec3 up;
  glm::vec3 right;
  bool fps;

public:
  Camera(glm::vec3 position = glm::vec3(0.0))
      : position(position), forward(0.0f, 0.0f,  1.0f), up(0.0f, -1.0f, 0.0f),
        right(1.0f, 0.0f, 0.0f), fps(false){};

  void pitch(float angle) {
    if (fps) {
      const float maxRot = M_PI / 2.0f - 0.01f;
      if (angle + rotation.x > maxRot || angle + rotation.x < -maxRot) {
        angle = glm::sign(rotation.x) * maxRot - rotation.x;
        rotation.x = glm::sign(rotation.x) * maxRot;
      } else
        rotation.x += angle;
      forward = glm::normalize(forward * cosf(angle) +
                               -glm::normalize(glm::cross(forward, right)) * sinf(angle));
    } else {
      rotation.x += angle;
      forward = glm::normalize(forward * cosf(angle) + up * sinf(angle));
      up = -glm::cross(forward, right);
    }
  }
  void yaw(float angle) {
    rotation.y += angle;
    if (fps)
      angle *= std::abs(glm::length(glm::cross(forward, up)));
    forward = glm::normalize(forward * cosf(angle) - right * sinf(angle));
    right = glm::normalize(glm::cross(forward, up));
  }
  void roll(const float angle) {
    rotation.z += angle;
    right = glm::normalize(right * cosf(angle) + up * sinf(angle));
    up = -glm::normalize(glm::cross(forward, right));
  }
  // z movement
  void advance(const float distance) { this->position += (forward * -distance); }

  // y movement
  void ascend(const float distance) { this->position += (up * distance); }

  // x movement
  void strafe(const float distance) { this->position += (right * distance); }

  void setFps(bool fps) {
    if (fps && !this->fps)
      rotation = glm::vec3();
    this->fps = fps;
  }

  bool getFps() { return fps; }

  glm::mat4 getViewMatrix() const {
    return glm::inverse(glm::lookAt(position, position + forward, up));
  }
};
