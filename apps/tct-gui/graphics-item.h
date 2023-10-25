#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <vector>

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

/**
 * @brief construct block for the graphic scene.
 *
 */
struct GraphicsItem {
  GraphicsItem(GraphicsItem* parent = nullptr);
  ~GraphicsItem();
  /**
   * @brief Set the transformation matrix
   * T = [[sx, 0, dx],
   *      [0, sy, dy],
   *      [0, 0, 1]];
   * points = [[x0,x1,x2,...],
   *          [y0,y1,y2,...],
   *          [1,1,1,...]]
   * points' = T * points
   *
   * @param matrix
   */
  void setTransform(Transform T);

  /**
   * @brief apply transformation T about the origin
   *
   * @param T
   * @param origin
   */
  void setTransform(Transform T, ImVec2 origin);

  /**
   * @brief translate the object and all the children in it's local coordinate
   * system.
   *
   * @param dx
   * @param dy
   */
  void setTranslation(float dx, float dy);
  /**
   * @brief scale the object and all the children in it's local coordinate
   * system.
   *
   * @param sx
   * @param sy
   */
  void setScale(float sx, float sy);
  /**
   * @brief clip the object and all the children in it's local coordinate
   * system.
   *
   * @param r
   */
  // void setClipRect(ImRect r);

  /**
   * @brief set the position of the item (center of is bounding box) in the
   * parent's coordinate system.
   *
   * @param p
   */
  void setPos(ImVec2 p);
  /**
   * @brief Set the transformation origin in the local coordinate system.
   *
   * @param p
   */
  void setTransformationOrigin(ImVec2 p);
  /**
   * @brief map a point from local coordinate system to parent's coordinate.
   *
   * @param p
   * @return ImVec2
   */
  ImVec2 mapToParent(ImVec2 p);
  /**
   * @brief map a point from parent's coordinate system to local coordinate.
   *
   * @param p
   * @return ImVec2
   */
  ImVec2 mapFromParent(ImVec2 p);
  /**
   * @brief map a point from local coordinate system to scene's coordinate.
   *
   * @param p
   * @return ImVec2
   */
  ImVec2 mapToScene(ImVec2 p);
  /**
   * @brief map a point from scene's coordinate system to local coordinate.
   *
   * @param p
   * @return ImVec2
   */
  ImVec2 mapFromScene(ImVec2 p);
  /**
   * @brief add a child to this item.
   *
   * @param child
   */
  void addChild(GraphicsItem* child);
  /**
   * @brief remove a child from this item.
   *
   * @param child
   */
  void removeChild(GraphicsItem* child);
  /**
   * @brief update the geometries of the item and all the children.
   *
   */
  void update();

  /**
   * @brief call the imgui function to draw the item and all the children.
   *
   */
  virtual void paint() = 0;
  virtual void contextMenu() = 0;
  virtual void focusInEvent() = 0;
  virtual void focusOutEvent() = 0;
  virtual void hoverEnterEvent() = 0;
  virtual void hoverLeaveEvent() = 0;
  virtual void doubleClickEvent() = 0;
  virtual void wheelEvent() = 0;

  GraphicsItem* parent_;
  std::vector<GraphicsItem*> children_;
  Transform T_;
  std::vector<ImVec2> geometries_;
  std::vector<ImVec2> sceneGeometries_;
  ImVec2 pos_;
  ImVec2 origin_;
  // ImRect clipRect_;
  // ImRect boundingRect_;
  ImColor lineColor_;
  ImColor fillColor_;
  float lineWidth_;
};

typedef std::vector<ImVec2> Polygon;

// struct GraphicsItemLine : public GraphicsItem {
//   std::pair<ImVec2, ImVec2> points;
// };

// struct GraphicsItemPolygon : public GraphicsItem {
//   std::vector<ImVec2> points;
// };

// struct GraphicsItemRect : public GraphicsItemPolygon {};

// here only for testing
bool lineClip(float x1, float y1, float x2, float y2, float xmin, float ymin,
              float xmax, float ymax, float& x1_out, float& y1_out,
              float& x2_out, float& y2_out);
bool lineClip(ImVec2 p1, ImVec2 p2, ImRect r, ImVec2& p1_out, ImVec2& p2_out);
Polygon polygonClip(Polygon points, ImRect r);
