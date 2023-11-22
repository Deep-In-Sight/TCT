#include "graphics-item.h"

#include <algorithm>
#include <iostream>

#include "utility.h"

#define UNCLIP_RECT ImRect(-1e4, -1e4, 1e4, 1e4)

GraphicsItem::GraphicsItem(const std::string& name) {
  name_ = name;
  parent_ = nullptr;

  T_ = Transform(1.0f, 1.0f, 0.0f, 0.0f);
  pos_ = ImVec2(0.0f, 0.0f);
  origin_ = ImVec2(0.0f, 0.0f);
  clipRect_ = UNCLIP_RECT;

  lineColor_ = IM_COL32(255, 255, 255, 255);
  fillColor_ = IM_COL32(255, 255, 255, 255);
  lineWidth_ = 1.0f;

  isClickable_ = true;
  isHidden_ = false;
}

GraphicsItem::~GraphicsItem() {}

void GraphicsItem::transform(Transform T) {
  T_ = T;
  update();
}

void GraphicsItem::transform(Transform T, ImVec2 origin) {
  auto oldOrigin = origin_;
  origin_ = origin;
  transform(T);
  origin_ = oldOrigin;
}

void GraphicsItem::translate(float dx, float dy, bool additive) {
  Transform T1(1.0f, 1.0f, -origin_.x, -origin_.y);
  Transform T2(1.0f, 1.0f, origin_.x, origin_.y);
  Transform T(1.0f, 1.0f, dx, dy);
  T_ = (additive) ? T2 * T * T1 * T_ : T2 * T * T1;
  update();
}

void GraphicsItem::scale(float sx, float sy, bool multiplicative) {
  Transform T1(1.0f, 1.0f, -origin_.x, -origin_.y);
  Transform T2(1.0f, 1.0f, origin_.x, origin_.y);
  Transform T(sx, sy, 0.0f, 0.0f);
  T_ = (multiplicative) ? T2 * T * T1 * T_ : T2 * T * T1;
  update();
}

void GraphicsItem::clip(ImRect r) {
  clipRect_ = r;
  update();
}

void GraphicsItem::unclip() { clip(UNCLIP_RECT); }

void GraphicsItem::setPos(ImVec2 pos) {
  pos_ = pos;
  update();
}

void GraphicsItem::setOrigin(ImVec2 p) {
  origin_ = p;
  // update();
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
  auto itemPos = mapToScene(ImVec2(0.0f, 0.0f));
  return p - itemPos;
}

ImVec2 GraphicsItem::mapToItem(GraphicsItem* item, ImVec2 p) {
  return item->mapFromScene(mapToScene(p));
}

ImVec2 GraphicsItem::mapFromItem(GraphicsItem* item, ImVec2 p) {
  return mapFromScene(item->mapToScene(p));
}

void GraphicsItem::addChild(GraphicsItemPtr child) {
  children_.push_back(child);
  child->parent_ = this;
  child->update();
}

void GraphicsItem::removeChild(GraphicsItemPtr child) {
  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    children_.erase(it);
    child->parent_ = nullptr;
  }
}

/**
 * @brief update the scene geometries of this item and its children.
 * For each item, calculate the acumulated transformation matrix, from itself up
 * to all the parents. Then calculate the scene geometries. Then move on to the
 * children and repeat. So probably a lot of redundant calculations. Would be a
 * problem if the number of item grows to a few thoudsands (and a lot of
 * hierachy levels), but for a couple of rects and lines with 2 geometries each,
 * just do it the readable way, instead of some ingenious nobody-can-understand
 * way.
 *
 */
void GraphicsItem::update() {
  // skip updating the geometries if this item is hidden (for itself and all the
  // children)
  if (isHidden_) {
    return;
  }

  // 1. calculate the combined transformation matrix
  Transform T;  // I
  GraphicsItem* item = this;
  // combine all the transformations from this item up to the root
  while (item != nullptr) {
    // Transform T1(1.0f, 1.0f, -item->origin_.x, -item->origin_.y);
    // Transform T2(1.0f, 1.0f, item->origin_.x, item->origin_.y);
    Transform Tpos(1.0f, 1.0f, item->pos_.x, item->pos_.y);
    // translate -origin, apply item's T_, move back origin, then move to pos in
    // parent
    // if (item->accumulateTransform_) {
    //   item->Tlast_ = T2 * item->T_ * T1 * item->Tlast_;
    //   // to prevent further accumulation in next children
    //   item->accumulateTransform_ = false;
    // } else {
    //   item->Tlast_ = T2 * item->T_ * T1;
    // }
    T = Tpos * item->T_ * T;
    item = item->parent_;
  }

  // 2. calculate the geometries
  sceneGeometries_.clear();
  for (auto p : geometries_) {
    sceneGeometries_.push_back(T * p);
  }

  // 3. clip the geometries
  item = this;
  ImRect R = UNCLIP_RECT;
  // overlap all the clipping rects from this item up to the root
  while (item != nullptr) {
    ImRect itemRect = item->clipRect_;
    itemRect.Translate(item->mapToScene(ImVec2(0.0f, 0.0f)));
    R.ClipWithFull(itemRect);
    item = item->parent_;
  }
  clipSelf(R);

  // 4. update the children
  for (auto child : children_) {
    child->update();
  }
}

void GraphicsItem::paint() {
  if (isHidden_) return;

  paintBegin();

  paintSelf();
  for (auto child : children_) {
    child->paint();
  }

  paintEnd();
}

void GraphicsItem::paintSelf() {}
void GraphicsItem::paintBegin() {}
void GraphicsItem::paintEnd() {}

void GraphicsItem::clipSelf(ImRect r) {}

bool GraphicsItem::hitTest(ImVec2 p) {
  // empty item cannot be hit
  return false;
}

GraphicsItemPtr GraphicsItem::operator[](int index) {
  if (index < 0 || index >= children_.size()) {
    return nullptr;
  }
  return children_[index];
}

GraphicsItemPtr findItem(GraphicsItemPtr item, ImVec2 point) {
  // disable hit test for itself and all its children
  if (!item->isClickable_ || item->isHidden_) {
    return nullptr;
  }

  GraphicsItem* ret = nullptr;
  // test young children first
  for (auto it = item->children_.rbegin(); it != item->children_.rend(); it++) {
    GraphicsItemPtr ret = findItem(*it, point);
    if (ret != nullptr) {
      return ret;
    }
  }
  // if child not found, test itself
  return item->hitTest(point) ? item : GraphicsItemPtr();
};

GraphicsItemPtr findItem(GraphicsItemPtr item, const std::string& name) {
  if (item->name_ == name) {
    return item;
  }
  for (auto child : item->children_) {
    GraphicsItemPtr ret = findItem(child, name);
    if (ret != nullptr) {
      return ret;
    }
  }
  return GraphicsItemPtr();
};

float intersect_with_yline(float x1, float y1, float x2, float y2, float y) {
  return x1 + (x2 - x1) * (y - y1) / (y2 - y1);
};

float intersect_with_xline(float x1, float y1, float x2, float y2, float x) {
  return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
};

/**
 * @brief Cohen-Sutherland line clipping algorithm
 * does it have to be this long for something so simple? yes it does.
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

/**
 * @brief ray casting algorithm, draw a horizontal line goes through p, and
 * count number of intersections with the polygon, on the right side of p. Point
 * p is inside the polygon if it lies on one the the line segments, of if number
 * of intersections is odd.
 *
 * @param points
 * @param p
 * @return true
 * @return false
 */
bool polygonContain(Polygon points, ImVec2 p) {
  int n = points.size();
  int i, j;
  int count = 0;
  for (i = 0, j = n - 1; i < n; j = i++) {
    float x1 = points[i].x;
    float y1 = points[i].y;
    float x2 = points[j].x;
    float y2 = points[j].y;
    float xc = intersect_with_yline(x1, y1, x2, y2, p.y);
    float yc = p.y;
    if (xc == p.x) {
      return true;
    }
    // intersection is on the right of p
    if (((y1 > yc) != (y2 > yc)) && (xc > p.x)) count++;
  }
  return count % 2 == 1;
}