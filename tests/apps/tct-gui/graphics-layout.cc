#include <graphics-layout.h>
#include <gtest/gtest.h>

// somebody write more tests please...

TEST(GraphicsLayout, TestHorizontalLayouts) {
  auto root = std::make_shared<GraphicsHBoxLayout>("root");
  auto child1 = std::make_shared<GraphicsHBoxLayout>("child1");
  auto child11 = std::make_shared<GraphicsHBoxLayout>("child11");
  auto child12 = std::make_shared<GraphicsHBoxLayout>("child12");
  auto child121 = std::make_shared<GraphicsHBoxLayout>("child121");
  auto child2 = std::make_shared<GraphicsHBoxLayout>("child2");

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

  std::cout << root->toString();
}

TEST(GraphicsLayout, TestVerticalLayouts) {
  auto root = std::make_shared<GraphicsVBoxLayout>("root");
  auto child1 = std::make_shared<GraphicsVBoxLayout>("child1");
  auto child11 = std::make_shared<GraphicsVBoxLayout>("child11");
  auto child12 = std::make_shared<GraphicsVBoxLayout>("child12");
  auto child121 = std::make_shared<GraphicsVBoxLayout>("child121");
  auto child2 = std::make_shared<GraphicsVBoxLayout>("child2");

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

  std::cout << root->toString();
}

TEST(GraphicsLayout, TestMixedLayouts) {
  auto root = std::make_shared<GraphicsHBoxLayout>("root");
  auto child1 = std::make_shared<GraphicsVBoxLayout>("child1");
  auto child11 = std::make_shared<GraphicsHBoxLayout>("child11");
  auto child12 = std::make_shared<GraphicsHBoxLayout>("child12");
  auto child121 = std::make_shared<GraphicsVBoxLayout>("child121");
  auto child2 = std::make_shared<GraphicsVBoxLayout>("child2");

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
  std::cout << root->toString();
}

TEST(GraphicsLayout, TestGridLayout) {
  auto root = std::make_shared<GraphicsGridLayout>(2, 2, "root");
  auto cell00 = std::make_shared<GraphicsHBoxLayout>("cell00");
  auto cell01 = std::make_shared<GraphicsVBoxLayout>("cell01");
  auto cell10 = std::make_shared<GraphicsVBoxLayout>("cell10");
  auto cell11 = std::make_shared<GraphicsHBoxLayout>("cell11");
  auto child000 = std::make_shared<GraphicsHBoxLayout>("child000");
  auto child001 = std::make_shared<GraphicsHBoxLayout>("child001");
  auto child010 = std::make_shared<GraphicsHBoxLayout>("child010");
  auto child011 = std::make_shared<GraphicsHBoxLayout>("child011");
  auto child100 = std::make_shared<GraphicsHBoxLayout>("child100");
  auto child101 = std::make_shared<GraphicsHBoxLayout>("child101");
  auto child110 = std::make_shared<GraphicsHBoxLayout>("child110");
  auto child111 = std::make_shared<GraphicsHBoxLayout>("child111");

  root->addChild(cell00);
  root->addChild(cell01);
  root->addChild(cell10);
  root->addChild(cell11);
  cell00->addChild(child000);
  cell00->addChild(child001);
  cell01->addChild(child010);
  cell01->addChild(child011);
  cell10->addChild(child100);
  cell10->addChild(child101);
  cell11->addChild(child110);
  cell11->addChild(child111);

  root->preferredSize_ = ImVec2(0, 0);  // auto
  root->margins_ = ImVec4(1, 1, 1, 1);

  cell00->preferredSize_ = ImVec2(0, 0);  // auto
  cell00->margins_ = ImVec4(1, 1, 1, 1);
  cell01->preferredSize_ = ImVec2(0, 0);  // auto
  cell01->margins_ = ImVec4(1, 1, 1, 1);
  cell10->preferredSize_ = ImVec2(0, 0);  // auto
  cell10->margins_ = ImVec4(1, 1, 1, 1);
  cell11->preferredSize_ = ImVec2(0, 0);  // auto
  cell11->margins_ = ImVec4(1, 1, 1, 1);

  child000->preferredSize_ = ImVec2(10, 10);
  child000->margins_ = ImVec4(0, 0, 0, 0);
  child001->preferredSize_ = ImVec2(10, 10);
  child001->margins_ = ImVec4(0, 0, 0, 0);

  child010->preferredSize_ = ImVec2(10, 10);
  child010->margins_ = ImVec4(0, 0, 0, 0);
  child011->preferredSize_ = ImVec2(10, 10);
  child011->margins_ = ImVec4(0, 0, 0, 0);

  child100->preferredSize_ = ImVec2(10, 10);
  child100->margins_ = ImVec4(0, 0, 0, 0);
  child101->preferredSize_ = ImVec2(10, 10);
  child101->margins_ = ImVec4(0, 0, 0, 0);

  child110->preferredSize_ = ImVec2(10, 10);
  child110->margins_ = ImVec4(0, 0, 0, 0);
  child111->preferredSize_ = ImVec2(10, 10);
  child111->margins_ = ImVec4(0, 0, 0, 0);

  root->layout();

  // leaf to root: 000->001->00->010->011->01->100->101->10->110->111->11->root
  EXPECT_EQ(child000->pos_.x, 1);  // cell00 has 1111 margin
  EXPECT_EQ(child000->pos_.y, 1);
  EXPECT_EQ(child000->preferredSize_.x, 10);
  EXPECT_EQ(child000->preferredSize_.y, 10);

  EXPECT_EQ(child001->pos_.x, 11);  // + child000 pos+size
  EXPECT_EQ(child001->pos_.y, 1);
  EXPECT_EQ(child001->preferredSize_.x, 10);
  EXPECT_EQ(child001->preferredSize_.y, 10);

  EXPECT_EQ(cell00->pos_.x, 1);  // root has 1111 margin
  EXPECT_EQ(cell00->pos_.y, 1);
  EXPECT_EQ(cell00->preferredSize_.x,
            22);  // auto: child000 size + child001 + margin
  EXPECT_EQ(cell00->preferredSize_.y, 12);

  EXPECT_EQ(child010->pos_.x, 1);  // cell01 has 1111 margin
  EXPECT_EQ(child010->pos_.y, 1);
  EXPECT_EQ(child010->preferredSize_.x, 10);
  EXPECT_EQ(child010->preferredSize_.y, 10);

  EXPECT_EQ(child011->pos_.x, 1);
  EXPECT_EQ(child011->pos_.y, 11);  // + child010 pos+size
  EXPECT_EQ(child011->preferredSize_.x, 10);
  EXPECT_EQ(child011->preferredSize_.y, 10);

  EXPECT_EQ(cell01->pos_.x, 23);  // + cell00 pos+size
  EXPECT_EQ(cell01->pos_.y, 1);
  EXPECT_EQ(cell01->preferredSize_.x, 12);
  EXPECT_EQ(cell01->preferredSize_.y,
            22);  // auto: child010 size + child011 + margin

  EXPECT_EQ(child100->pos_.x, 1);  // cell10 has 1111 margin
  EXPECT_EQ(child100->pos_.y, 1);
  EXPECT_EQ(child100->preferredSize_.x, 10);
  EXPECT_EQ(child100->preferredSize_.y, 10);

  EXPECT_EQ(child101->pos_.x, 1);
  EXPECT_EQ(child101->pos_.y, 11);  // + child100 pos+size
  EXPECT_EQ(child101->preferredSize_.x, 10);
  EXPECT_EQ(child101->preferredSize_.y, 10);

  EXPECT_EQ(cell10->pos_.x, 1);
  EXPECT_EQ(cell10->pos_.y, 23);  // + cell00 pos + max height
  EXPECT_EQ(cell10->preferredSize_.x, 12);
  EXPECT_EQ(cell10->preferredSize_.y,
            22);  // auto: child100 size + child101 + margin

  EXPECT_EQ(child110->pos_.x, 1);  // cell11 has 1111 margin
  EXPECT_EQ(child110->pos_.y, 1);
  EXPECT_EQ(child110->preferredSize_.x, 10);
  EXPECT_EQ(child110->preferredSize_.y, 10);

  EXPECT_EQ(child111->pos_.x, 11);  // + child110 pos+size
  EXPECT_EQ(child111->pos_.y, 1);
  EXPECT_EQ(child111->preferredSize_.x, 10);
  EXPECT_EQ(child111->preferredSize_.y, 10);

  EXPECT_EQ(cell11->pos_.x, 23);  // + cell10 pos+ max width
  EXPECT_EQ(cell11->pos_.y, 23);  // + cell01 pos+ max height
  EXPECT_EQ(cell11->preferredSize_.x, 22);
  EXPECT_EQ(cell11->preferredSize_.y, 12);

  EXPECT_EQ(root->pos_.x, 0);
  EXPECT_EQ(root->pos_.y, 0);
  EXPECT_EQ(root->preferredSize_.x, 46);
  EXPECT_EQ(root->preferredSize_.y, 46);

  std::cout << root->toString();
}