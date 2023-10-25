#include <graphics-item.h>
#include <gtest/gtest.h>

#include <vector>
TEST(GraphicsItem, TestLineClipping) {
  float xmin = 0.0f;
  float xmax = 100.0f;
  float ymin = 0.0f;
  float ymax = 100.0f;
  float x1_out, y1_out, x2_out, y2_out;
  // 10 points, 2 in, 8 out
  /*
     7    8    9
       ┌──────┐
       │      │
     3 │  45  │6
       ▲      │
       └►─────┘
     0    1    2
   */
  float x[] = {-10.0f, 50.0f,  110.0f, -10.0f, 50.0f,
               60.0f,  110.0f, -10.0f, 50.0f,  110.0f};
  float y[] = {-10.0f, -10.0f, -10.0f, 50.0f,  50.0f,
               50.0f,  50.0f,  110.0f, 110.0f, 110.0f};
  // 1. completely inside
  int start = 4, end = 5;
  EXPECT_TRUE(lineClip(x[start], y[start], x[end], y[end], xmin, ymin, xmax,
                       ymax, x1_out, y1_out, x2_out, y2_out));
  EXPECT_FLOAT_EQ(x[start], x1_out);
  EXPECT_FLOAT_EQ(y[start], y1_out);
  EXPECT_FLOAT_EQ(x[end], x2_out);
  EXPECT_FLOAT_EQ(y[end], y2_out);
  // 2. completely outside
  std::vector<int> starts = {0, 7, 0, 2};
  std::vector<int> ends = {2, 9, 7, 9};
  for (int i = 0; i < 4; i++) {
    start = starts[i];
    end = ends[i];
    EXPECT_FALSE(lineClip(x[start], y[start], x[end], y[end], xmin, ymin, xmax,
                          ymax, x1_out, y1_out, x2_out, y2_out));
  }
  // 3. partially inside
  starts = std::vector<int>({4, 4, 4, 4});
  ends = std::vector<int>({1, 3, 6, 8});
  std::vector<float> x1_outs = {50.0f, 50.0f, 50.0f, 50.0f};
  std::vector<float> y1_outs = {50.0f, 50.0f, 50.0f, 50.0f};
  std::vector<float> x2_outs = {50.0f, 0.0f, 100.0f, 50.0f};
  std::vector<float> y2_outs = {0.0f, 50.0f, 50.0f, 100.0f};
  for (int i = 0; i < 4; i++) {
    start = starts[i];
    end = ends[i];
    EXPECT_TRUE(lineClip(x[start], y[start], x[end], y[end], xmin, ymin, xmax,
                         ymax, x1_out, y1_out, x2_out, y2_out));
    EXPECT_FLOAT_EQ(x1_outs[i], x1_out);
    EXPECT_FLOAT_EQ(y1_outs[i], y1_out);
    EXPECT_FLOAT_EQ(x2_outs[i], x2_out);
    EXPECT_FLOAT_EQ(y2_outs[i], y2_out);
  }
  // 4. both ends outside, but crosses the window
  starts = std::vector<int>({1, 3});
  ends = std::vector<int>({8, 6});
  x1_outs = std::vector<float>({50.0f, 0.0f});
  y1_outs = std::vector<float>({0.0f, 50.0f});
  x2_outs = std::vector<float>({50.0f, 100.0f});
  y2_outs = std::vector<float>({100.0f, 50.0f});
  for (int i = 0; i < 2; i++) {
    start = starts[i];
    end = ends[i];
    EXPECT_TRUE(lineClip(x[start], y[start], x[end], y[end], xmin, ymin, xmax,
                         ymax, x1_out, y1_out, x2_out, y2_out));
    EXPECT_FLOAT_EQ(x1_outs[i], x1_out);
    EXPECT_FLOAT_EQ(y1_outs[i], y1_out);
    EXPECT_FLOAT_EQ(x2_outs[i], x2_out);
    EXPECT_FLOAT_EQ(y2_outs[i], y2_out);
  }
}

TEST(GraphicsItem, TestPolygonClip) {
  Polygon subjectPolygon;
  subjectPolygon.push_back(ImVec2(-10, -10));
  subjectPolygon.push_back(ImVec2(10, -10));
  subjectPolygon.push_back(ImVec2(10, 10));
  subjectPolygon.push_back(ImVec2(30, 10));
  subjectPolygon.push_back(ImVec2(30, -10));
  subjectPolygon.push_back(ImVec2(60, -10));
  subjectPolygon.push_back(ImVec2(60, 10));
  subjectPolygon.push_back(ImVec2(40, 10));
  subjectPolygon.push_back(ImVec2(40, 20));
  subjectPolygon.push_back(ImVec2(60, 20));
  subjectPolygon.push_back(ImVec2(60, 50));
  subjectPolygon.push_back(ImVec2(40, 50));
  subjectPolygon.push_back(ImVec2(40, 30));
  subjectPolygon.push_back(ImVec2(10, 30));
  subjectPolygon.push_back(ImVec2(10, 50));
  subjectPolygon.push_back(ImVec2(-10, 50));

  // Define the clipping window
  ImRect r(0, 0, 50, 40);
  Polygon expectPoints;
  expectPoints.push_back(ImVec2(0, 40));
  expectPoints.push_back(ImVec2(0, 0));
  expectPoints.push_back(ImVec2(10, 0));
  expectPoints.push_back(ImVec2(10, 10));
  expectPoints.push_back(ImVec2(30, 10));
  expectPoints.push_back(ImVec2(30, 0));
  expectPoints.push_back(ImVec2(50, 0));
  expectPoints.push_back(ImVec2(50, 10));
  expectPoints.push_back(ImVec2(40, 10));
  expectPoints.push_back(ImVec2(40, 20));
  expectPoints.push_back(ImVec2(50, 20));
  expectPoints.push_back(ImVec2(50, 40));
  expectPoints.push_back(ImVec2(40, 40));
  expectPoints.push_back(ImVec2(40, 30));
  expectPoints.push_back(ImVec2(10, 30));
  expectPoints.push_back(ImVec2(10, 40));

  // Perform polygon clipping
  Polygon clippedPolygon = polygonClip(subjectPolygon, r);
  EXPECT_EQ(expectPoints.size(), clippedPolygon.size());
  for (int i = 0; i < clippedPolygon.size(); i++) {
    EXPECT_FLOAT_EQ(expectPoints[i].x, clippedPolygon[i].x);
    EXPECT_FLOAT_EQ(expectPoints[i].y, clippedPolygon[i].y);
  }
}