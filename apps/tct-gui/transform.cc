#include "transform.h"

Transform::Transform(float sx, float sy, float dx, float dy) {
  sx_ = sx;
  sy_ = sy;
  dx_ = dx;
  dy_ = dy;
}

Transform::Transform() {
  sx_ = 1.0f;
  sy_ = 1.0f;
  dx_ = 0.0f;
  dy_ = 0.0f;
}

bool Transform::isIdentity() {
  return sx_ == 1.0f && sy_ == 1.0f && dx_ == 0.0f && dy_ == 0.0f;
}

Transform Transform::operator*(const Transform& rhs) {
  Transform ret;
  ret.sx_ = sx_ * rhs.sx_;
  ret.sy_ = sy_ * rhs.sy_;
  ret.dx_ = sx_ * rhs.dx_ + dx_;
  ret.dy_ = sy_ * rhs.dy_ + dy_;
  return ret;
}

ImVec2 Transform::operator*(const ImVec2& rhs) {
  ImVec2 ret;
  ret.x = sx_ * rhs.x + dx_;
  ret.y = sy_ * rhs.y + dy_;
  return ret;
}
