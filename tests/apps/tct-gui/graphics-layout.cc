#include <graphics-layout.h>
#include <gtest/gtest.h>

// somebody write more tests please...

TEST(GraphicsLayout, TestHorizontalLayouts) {
  auto root = std::make_shared<GraphicsHBoxLayout>();
  auto child1 = std::make_shared<GraphicsHBoxLayout>();
  auto child11 = std::make_shared<GraphicsHBoxLayout>();
  auto child12 = std::make_shared<GraphicsHBoxLayout>();
  auto child121 = std::make_shared<GraphicsHBoxLayout>();
  auto child2 = std::make_shared<GraphicsHBoxLayout>();

  /*
  root
    ├──child1
    │     ├──child11
    │     └──child12
    │           └──child121
    └──child2
  */
  root->addChild(child1);
  root->addChild(child2);
  child1->addChild(child11);
  child1->addChild(child12);
  child12->addChild(child121);

  root->preferredSize_ = ImVec2(0, 0);  // auto
  root->margins_ = ImVec4(1, 1, 1, 1);

  child1->preferredSize_ = ImVec2(0, 0);  // auto
  child1->margins_ = ImVec4(0, 0, 0, 0);

  child11->preferredSize_ = ImVec2(10, 10);
  child11->margins_ = ImVec4(1, 1, 1, 1);

  child12->preferredSize_ = ImVec2(0, 0);  // auto
  child12->margins_ = ImVec4(1, 1, 1, 1);

  child121->preferredSize_ = ImVec2(20, 20);
  child121->margins_ = ImVec4(1, 1, 1, 1);

  child2->preferredSize_ = ImVec2(30, 30);
  child2->margins_ = ImVec4(0, 0, 0, 0);

  root->layout();

  // leaf to root: 11->121->12->1->2>root
  EXPECT_EQ(child11->pos_.x, 0);  // because child1 has no margin
  EXPECT_EQ(child11->pos_.y, 0);
  EXPECT_EQ(child11->preferredSize_.x, 10);
  EXPECT_EQ(child11->preferredSize_.y, 10);

  EXPECT_EQ(child121->pos_.x, 1);  // because child12 has 1111 margin
  EXPECT_EQ(child121->pos_.y, 1);
  EXPECT_EQ(child121->preferredSize_.x, 20);
  EXPECT_EQ(child121->preferredSize_.y, 20);

  EXPECT_EQ(child12->pos_.x, 10);  // + child11 size
  EXPECT_EQ(child12->pos_.y, 0);
  EXPECT_EQ(child12->preferredSize_.x,
            22);  // auto: child12 margin + child121 size
  EXPECT_EQ(child12->preferredSize_.y, 22);

  EXPECT_EQ(child1->pos_.x, 1);  // because root has 1111 margin
  EXPECT_EQ(child1->pos_.y, 1);
  EXPECT_EQ(child1->preferredSize_.x,
            32);                            // auto: child11 size + child12 size
  EXPECT_EQ(child1->preferredSize_.y, 22);  // auto: bigger size

  EXPECT_EQ(child2->pos_.x, 33);  // + child1 size
  EXPECT_EQ(child2->pos_.y, 1);
  EXPECT_EQ(child2->preferredSize_.x, 30);
  EXPECT_EQ(child2->preferredSize_.y, 30);

  EXPECT_EQ(root->pos_.x, 0);
  EXPECT_EQ(root->pos_.y, 0);
  EXPECT_EQ(root->preferredSize_.x,
            64);  // auto: root margin + child1 size + child2 size
  EXPECT_EQ(root->preferredSize_.y, 32);  // auto: bigger size
}

TEST(GraphicsLayout, TestVerticalLayouts) {
  auto root = std::make_shared<GraphicsVBoxLayout>();
  auto child1 = std::make_shared<GraphicsVBoxLayout>();
  auto child11 = std::make_shared<GraphicsVBoxLayout>();
  auto child12 = std::make_shared<GraphicsVBoxLayout>();
  auto child121 = std::make_shared<GraphicsVBoxLayout>();
  auto child2 = std::make_shared<GraphicsVBoxLayout>();

  /*
  root
    ├──child1
    │     ├──child11
    │     └──child12
    │           └──child121
    └──child2
  */
  root->addChild(child1);
  root->addChild(child2);
  child1->addChild(child11);
  child1->addChild(child12);
  child12->addChild(child121);

  root->preferredSize_ = ImVec2(0, 0);  // auto
  root->margins_ = ImVec4(1, 1, 1, 1);

  child1->preferredSize_ = ImVec2(0, 0);  // auto
  child1->margins_ = ImVec4(0, 0, 0, 0);

  child11->preferredSize_ = ImVec2(10, 10);
  child11->margins_ = ImVec4(1, 1, 1, 1);

  child12->preferredSize_ = ImVec2(0, 0);  // auto
  child12->margins_ = ImVec4(1, 1, 1, 1);

  child121->preferredSize_ = ImVec2(20, 20);
  child121->margins_ = ImVec4(1, 1, 1, 1);

  child2->preferredSize_ = ImVec2(30, 30);
  child2->margins_ = ImVec4(0, 0, 0, 0);

  root->layout();

  // leaf to root: 11->121->12->1->2>root
  EXPECT_EQ(child11->pos_.x, 0);  // because child1 has no margin
  EXPECT_EQ(child11->pos_.y, 0);
  EXPECT_EQ(child11->preferredSize_.x, 10);
  EXPECT_EQ(child11->preferredSize_.y, 10);

  EXPECT_EQ(child121->pos_.x, 1);  // because child12 has 1111 margin
  EXPECT_EQ(child121->pos_.y, 1);
  EXPECT_EQ(child121->preferredSize_.x, 20);
  EXPECT_EQ(child121->preferredSize_.y, 20);

  EXPECT_EQ(child12->pos_.x, 0);
  EXPECT_EQ(child12->pos_.y, 10);  // + child11 pos+size
  EXPECT_EQ(child12->preferredSize_.x, 22);
  EXPECT_EQ(child12->preferredSize_.y,
            22);  // auto: child12 margin + child121 size

  EXPECT_EQ(child1->pos_.x, 1);  // because root has 1111 margin
  EXPECT_EQ(child1->pos_.y, 1);
  EXPECT_EQ(child1->preferredSize_.x, 22);  // auto: bigger size
  EXPECT_EQ(child1->preferredSize_.y, 32);  // auto: child11 size + child12 size

  EXPECT_EQ(child2->pos_.x, 1);
  EXPECT_EQ(child2->pos_.y, 33);  // + child1 pos + size
  EXPECT_EQ(child2->preferredSize_.x, 30);
  EXPECT_EQ(child2->preferredSize_.y, 30);

  EXPECT_EQ(root->pos_.x, 0);
  EXPECT_EQ(root->pos_.y, 0);
  EXPECT_EQ(root->preferredSize_.x, 32);  // auto: bigger size
  EXPECT_EQ(root->preferredSize_.y,
            64);  // auto: root margin + child1 size + child2 size
}

TEST(GraphicsLayout, TestMixedLayouts) {
  auto root = std::make_shared<GraphicsHBoxLayout>();
  auto child1 = std::make_shared<GraphicsVBoxLayout>();
  auto child11 = std::make_shared<GraphicsHBoxLayout>();
  auto child12 = std::make_shared<GraphicsHBoxLayout>();
  auto child121 = std::make_shared<GraphicsVBoxLayout>();
  auto child2 = std::make_shared<GraphicsVBoxLayout>();

  /*
  root
    ├──child1
    │     ├──child11
    │     └──child12
    │           └──child121
    └──child2
  */
  root->addChild(child1);
  root->addChild(child2);
  child1->addChild(child11);
  child1->addChild(child12);
  child12->addChild(child121);

  root->preferredSize_ = ImVec2(0, 0);  // auto
  root->margins_ = ImVec4(1, 1, 1, 1);

  child1->preferredSize_ = ImVec2(0, 0);  // auto
  child1->margins_ = ImVec4(0, 0, 0, 0);

  child11->preferredSize_ = ImVec2(10, 10);
  child11->margins_ = ImVec4(1, 1, 1, 1);

  child12->preferredSize_ = ImVec2(0, 0);  // auto
  child12->margins_ = ImVec4(1, 1, 1, 1);

  child121->preferredSize_ = ImVec2(20, 20);
  child121->margins_ = ImVec4(1, 1, 1, 1);

  child2->preferredSize_ = ImVec2(30, 30);
  child2->margins_ = ImVec4(0, 0, 0, 0);

  root->layout();

  // leaf to root: 11->121->12->1->2>root
  EXPECT_EQ(child11->pos_.x, 0);  // because child1 has no margin
  EXPECT_EQ(child11->pos_.y, 0);
  EXPECT_EQ(child11->preferredSize_.x, 10);
  EXPECT_EQ(child11->preferredSize_.y, 10);

  EXPECT_EQ(child121->pos_.x, 1);  // because child12 has 1111 margin
  EXPECT_EQ(child121->pos_.y, 1);
  EXPECT_EQ(child121->preferredSize_.x, 20);
  EXPECT_EQ(child121->preferredSize_.y, 20);

  EXPECT_EQ(child12->pos_.x, 0);
  EXPECT_EQ(child12->pos_.y, 10);  // + child11 pos+size
  EXPECT_EQ(child12->preferredSize_.x, 22);
  EXPECT_EQ(child12->preferredSize_.y,
            22);  // auto: child12 margin + child121 size

  EXPECT_EQ(child1->pos_.x, 1);  // because root has 1111 margin
  EXPECT_EQ(child1->pos_.y, 1);
  EXPECT_EQ(child1->preferredSize_.x, 22);  // auto: bigger size
  EXPECT_EQ(child1->preferredSize_.y, 32);  // auto: child11 size + child12 size

  EXPECT_EQ(child2->pos_.x, 23);
  EXPECT_EQ(child2->pos_.y, 1);  // + child1 pos + size
  EXPECT_EQ(child2->preferredSize_.x, 30);
  EXPECT_EQ(child2->preferredSize_.y, 30);

  EXPECT_EQ(root->pos_.x, 0);
  EXPECT_EQ(root->pos_.y, 0);
  EXPECT_EQ(root->preferredSize_.x, 54);  // auto: bigger size
  EXPECT_EQ(root->preferredSize_.y,
            34);  // auto: root margin + child1 size + child2 size
}
