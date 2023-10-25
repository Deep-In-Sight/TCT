#include "graphics-item.h"

#include <algorithm>

float intersect_with_yline(float x1, float y1, float x2, float y2, float y) {
  return x1 + (x2 - x1) * (y - y1) / (y2 - y1);
};

float intersect_with_xline(float x1, float y1, float x2, float y2, float x) {
  return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
};

/**
 * @brief Cohen-Sutherland line clipping algorithm
 *
 * @return false if the line is completely outside the clipping window
 */
bool lineClip(float x1, float y1, float x2, float y2, float xmin, float ymin,
              float xmax, float ymax, float& x1_out, float& y1_out,
              float& x2_out, float& y2_out) {
#define INSIDE 0
#define LEFT 1
#define RIGHT 2
#define BOTTOM 4
#define TOP 8
  auto computeCode = [](float x, float y, float xmin, float ymin, float xmax,
                        float ymax) -> int {
    int code = INSIDE;
    if (x < xmin) {
      code |= LEFT;
    } else if (x > xmax) {
      code |= RIGHT;
    }
    if (y < ymin) {
      code |= BOTTOM;
    } else if (y > ymax) {
      code |= TOP;
    }
    return code;
  };

  int code1 = computeCode(x1, y1, xmin, ymin, xmax, ymax);
  int code2 = computeCode(x2, y2, xmin, ymin, xmax, ymax);
  bool accept = false;

  while (true) {
    if (!(code1 | code2)) {
      // Both endpoints are inside the clipping window
      accept = true;
      break;
    } else if (code1 & code2) {
      // Both endpoints are outside the same region
      break;
    } else {
      // Calculate intersection coordinates
      float x, y;

      int outcodeOut = code1 ? code1 : code2;

      if (outcodeOut & TOP) {
        // Line clips the top edge
        x = intersect_with_yline(x1, y1, x2, y2, ymax);
        y = ymax;
      } else if (outcodeOut & BOTTOM) {
        // Line clips the bottom edge
        x = intersect_with_yline(x1, y1, x2, y2, ymin);
        y = ymin;
      } else if (outcodeOut & RIGHT) {
        // Line clips the right edge
        y = intersect_with_xline(x1, y1, x2, y2, xmax);
        x = xmax;
      } else {
        // Line clips the left edge
        y = intersect_with_xline(x1, y1, x2, y2, xmin);
        x = xmin;
      }

      if (outcodeOut == code1) {
        x1 = x;
        y1 = y;
        code1 = computeCode(x1, y1, xmin, ymin, xmax, ymax);
      } else {
        x2 = x;
        y2 = y;
        code2 = computeCode(x2, y2, xmin, ymin, xmax, ymax);
      }
    }
  }
  x1_out = x1;
  y1_out = y1;
  x2_out = x2;
  y2_out = y2;

  return accept;
}

bool lineClip(ImVec2 p1, ImVec2 p2, ImRect r, ImVec2& p1_out, ImVec2& p2_out) {
  float x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y;
  float xmin = r.Min.x, ymin = r.Min.y, xmax = r.Max.x, ymax = r.Max.y;
  float x1_out, y1_out, x2_out, y2_out;
  bool ret = lineClip(x1, y1, x2, y2, xmin, ymin, xmax, ymax, x1_out, y1_out,
                      x2_out, y2_out);
  p1_out = ImVec2(x1_out, y1_out);
  p2_out = ImVec2(x2_out, y2_out);
  return ret;
}

/**
 * @brief Sutherland-Hodgman polygon clipping algorithm
 *
 * @param points
 * @param xmin
 * @param ymin
 * @param xmax
 * @param ymax
 * @param points_out
 * @return false if the polygon is completely outside the clipping window
 */
Polygon polygonClip(Polygon points, ImRect r) {
  Polygon inputPolygon = points;
  Polygon outputPolygon;

  // check against top edge
  ImVec2 S = inputPolygon.back();
  for (auto P : inputPolygon) {
    if (S.y < r.Max.y) {
      if (P.y < r.Max.y) {
        outputPolygon.push_back(P);
      } else {
        float x = intersect_with_yline(S.x, S.y, P.x, P.y, r.Max.y);
        outputPolygon.push_back(ImVec2(x, r.Max.y));
      }
    } else {
      if (P.y < r.Max.y) {
        float x = intersect_with_yline(S.x, S.y, P.x, P.y, r.Max.y);
        outputPolygon.push_back(ImVec2(x, r.Max.y));
        outputPolygon.push_back(P);
      }
    }
    S = P;
  }
  // check against right edge
  inputPolygon = outputPolygon;
  outputPolygon.clear();
  S = inputPolygon.back();
  for (auto P : inputPolygon) {
    if (S.x < r.Max.x) {
      if (P.x < r.Max.x) {
        outputPolygon.push_back(P);
      } else {
        float y = intersect_with_xline(S.x, S.y, P.x, P.y, r.Max.x);
        outputPolygon.push_back(ImVec2(r.Max.x, y));
      }
    } else {
      if (P.x < r.Max.x) {
        float y = intersect_with_xline(S.x, S.y, P.x, P.y, r.Max.x);
        outputPolygon.push_back(ImVec2(r.Max.x, y));
        outputPolygon.push_back(P);
      }
    }
    S = P;
  }
  // check against bottom edge
  inputPolygon = outputPolygon;
  outputPolygon.clear();
  S = inputPolygon.back();
  for (auto P : inputPolygon) {
    if (S.y > r.Min.y) {
      if (P.y > r.Min.y) {
        outputPolygon.push_back(P);
      } else {
        float x = intersect_with_yline(S.x, S.y, P.x, P.y, r.Min.y);
        outputPolygon.push_back(ImVec2(x, r.Min.y));
      }
    } else {
      if (P.y > r.Min.y) {
        float x = intersect_with_yline(S.x, S.y, P.x, P.y, r.Min.y);
        outputPolygon.push_back(ImVec2(x, r.Min.y));
        outputPolygon.push_back(P);
      }
    }
    S = P;
  }
  // check against left edge
  inputPolygon = outputPolygon;
  outputPolygon.clear();
  S = inputPolygon.back();
  for (auto P : inputPolygon) {
    if (S.x > r.Min.x) {
      if (P.x > r.Min.x) {
        outputPolygon.push_back(P);
      } else {
        float y = intersect_with_xline(S.x, S.y, P.x, P.y, r.Min.x);
        outputPolygon.push_back(ImVec2(r.Min.x, y));
      }
    } else {
      if (P.x > r.Min.x) {
        float y = intersect_with_xline(S.x, S.y, P.x, P.y, r.Min.x);
        outputPolygon.push_back(ImVec2(r.Min.x, y));
        outputPolygon.push_back(P);
      }
    }
    S = P;
  }

  return outputPolygon;
}

GraphicsItem::GraphicsItem(GraphicsItem* parent) {
  parent_ = parent;
  parent_->addChild(this);

  T_ = Transform(1.0f, 1.0f, 0.0f, 0.0f);
  pos_ = ImVec2(0.0f, 0.0f);
  origin_ = ImVec2(0.0f, 0.0f);
  // clipRect_ = ImRect(0.0f, 0.0f, 0.0f, 0.0f);
  // boundingRect_ = ImRect(0.0f, 0.0f, 0.0f, 0.0f);
  lineColor_ = IM_COL32(255, 255, 255, 255);
  fillColor_ = IM_COL32(255, 255, 255, 255);
  lineWidth_ = 1.0f;
}

GraphicsItem::~GraphicsItem() {
  for (auto child : children_) {
    delete child;
  }
}

void GraphicsItem::setTransform(Transform T) {
  T_ = T;
  update();
  for (auto child : children_) {
    auto T_backup = child->T_;
    auto origin_backup = child->origin_;

    auto parentOrigin = child->mapFromParent(origin_);
    child->setTransform(T, parentOrigin);
    child->setTransform(T_backup, origin_backup);
  }
}

void GraphicsItem::setTransform(Transform T, ImVec2 origin) {
  T.dx_ += origin.x * (1 - T.sx_);
  T.dy_ += origin.y * (1 - T.sy_);
  origin_ = origin;
  setTransform(T);
}

void GraphicsItem::setTranslation(float dx, float dy) {
  T_.dx_ = dx;
  T_.dx_ = dy;
  update();
}

void GraphicsItem::setScale(float sx, float sy) {
  T_.sx_ = sx;
  T_.sy_ = sy;
  update();
}

// void GraphicsItem::setClipRect(ImRect r) {
//   clipRect_ = r;
//   update();
// }

void GraphicsItem::setPos(ImVec2 pos) {
  pos_ = pos;
  update();
}

void GraphicsItem::setTransformationOrigin(ImVec2 origin) {
  /**
   * translate so that new origin is at (0, 0)
   * apply the transformation
   * then translate back
   *
   * update the matrix
   * M = T * M * T', with
   * T' = [[1, 0, -dx],
   *      [0, 1, -dy],
   *      [0, 0, 1]]
   * T = [[1, 0, dx],
   *      [0, 1, dy],
   *      [0, 0, 1]]
   * [dx, dy] = origin - origin_;
   */
  auto t = origin - origin_;
  T_.dx_ += t.x * (1 - T_.sx_);
  T_.dy_ += t.y * (1 - T_.sy_);
  origin_ = origin;
  update();
}

ImVec2 GraphicsItem::mapToParent(ImVec2 p) { return p + pos_; }

ImVec2 GraphicsItem::mapFromParent(ImVec2 p) { return p - pos_; }

ImVec2 GraphicsItem::mapToScene(ImVec2 p) {
  GraphicsItem* item = this;
  ImVec2 ret = p;
  while (item != nullptr) {
    ret = item->mapToParent(ret);
    item = item->parent_;
  }
  return ret;
}

ImVec2 GraphicsItem::mapFromScene(ImVec2 p) {
  GraphicsItem* item = this;
  ImVec2 ret = p;
  while (item != nullptr) {
    ret = item->mapFromParent(ret);
    item = item->parent_;
  }
  return ret;
}

void GraphicsItem::addChild(GraphicsItem* child) {
  children_.push_back(child);
  child->parent_ = this;
}

void GraphicsItem::removeChild(GraphicsItem* child) {
  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    children_.erase(it);
    child->parent_ = nullptr;
  }
}

void GraphicsItem::update() {
  sceneGeometries_.clear();

  for (auto p : geometries_) {
    float x = p.x * T_.sx_ + T_.dx_;
    float y = p.y * T_.sy_ + T_.dy_;
    ImVec2 scenePoint = mapToScene(ImVec2(x, y));
    sceneGeometries_.push_back(scenePoint);
  }
}