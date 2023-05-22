#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/core/base-sink.h>
#include <sdk/core/base-src.h>
#include <sdk/core/base-transform.h>

class BaseSourceMock : public BaseSource {
 public:
  BaseSourceMock(const string& name) : BaseSource(name) {}
  ~BaseSourceMock() {}

  MOCK_METHOD(cv::Mat, GenerateFrame, (), (override));
};

class BaseSourceTest : public ::testing::Test {
 protected:
  BaseSourceTest() { test_src_ = new BaseSourceMock("cam0"); }
  ~BaseSourceTest() override { delete test_src_; }

  BaseSourceMock* test_src_;
};

TEST_F(BaseSourceTest, TestBaseSourceContructor) {
  test_src_->SetName("cam");
  EXPECT_TRUE(test_src_->GetName() == "cam");
  EXPECT_NE(test_src_->GetPad("src"), nullptr);
}

TEST_F(BaseSourceTest, TestGenerateLoop) {
  EXPECT_CALL(*test_src_, GenerateFrame).Times(::testing::AtLeast(1));
  test_src_->Start();
  EXPECT_TRUE(test_src_->IsRunning());
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  test_src_->Stop();
  EXPECT_FALSE(test_src_->IsRunning());
}
