#pragma once

#include <imgui.h>

/**
 * @brief represent a simple scale and translation transformation matrix.
 * [[sx, 0, dx],
 * [0, sy, dy],
 * [0, 0, 1]];
 */
struct Transform {
  Transform();
  Transform(float sx, float sy, float dx, float dy);
  bool isIdentity();
  Transform operator*(const Transform& rhs);
  ImVec2 operator*(const ImVec2& rhs);
  float sx_, sy_, dx_, dy_;
};
