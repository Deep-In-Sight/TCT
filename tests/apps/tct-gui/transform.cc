#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <transform.h>

class TransformTest : public ::testing::Test {
 protected:
  void SetUp() override { transform_ = new Transform(1.0f, 1.0f, 0.0f, 0.0f); }
  void TearDown() override { delete transform_; }
  Transform *transform_;
};

TEST_F(TransformTest, TestIsIdentity) { EXPECT_TRUE(transform_->isIdentity()); }

TEST_F(TransformTest, TestMultiply) {
  Transform t1(1.0f, 1.0f, 0.0f, 0.0f);
  Transform t2(1.0f, 1.0f, 0.0f, 0.0f);
  Transform t3 = t1 * t2;
  EXPECT_TRUE(t3.isIdentity());

  Transform t4(2.0f, 2.0f, 3.0f, 3.0f);
  Transform t5 = t1 * t4;
  EXPECT_EQ(t5.sx_, 2.0f);
  EXPECT_EQ(t5.sy_, 2.0f);
  EXPECT_EQ(t5.dx_, 3.0f);
  EXPECT_EQ(t5.dy_, 3.0f);

  ImVec2 v1(1.0f, 1.0f);
  ImVec2 v2 = t4 * v1;
  EXPECT_EQ(v2.x, 5.0f);
  EXPECT_EQ(v2.y, 5.0f);
}