#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/core/base-sink.h>
#include <sdk/core/base-src.h>
#include <sdk/core/base-transform.h>

using ::testing::NiceMock;

class BaseSourceMock : public BaseSource {
 public:
  BaseSourceMock(const string& name) : BaseSource(name) {}
  ~BaseSourceMock() {
    // if child is destroyed while base thread is running, pure virtual methods
    // in base class may be called
    Stop();
  }

  MOCK_METHOD(cv::Mat, GenerateFrame, (), (override));
  MOCK_METHOD(void, InitializeSource, (), (override));
  MOCK_METHOD(void, CleanupSource, (), (override));
};

class BaseSourceTest : public ::testing::Test {
 protected:
  void SetUp() override { test_src_ = new NiceMock<BaseSourceMock>("cam"); }
  void TearDown() override { delete test_src_; }

  NiceMock<BaseSourceMock>* test_src_;
};

TEST_F(BaseSourceTest, TestBaseSourceContructor) {
  test_src_->SetName("cam");
  EXPECT_TRUE(test_src_->GetName() == "cam");
  EXPECT_NE(test_src_->GetPad("src"), nullptr);
}

// test the state machine in the controlling thread only. The processing thread
// where the mocked functions are called may or may not happen, depends on
// system scheduling.
TEST_F(BaseSourceTest, TestStartFromStopped) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_TRUE(test_src_->Stop());
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
}

TEST_F(BaseSourceTest, TestStartFromPlaying) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_FALSE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
}

TEST_F(BaseSourceTest, TestStartFromPaused) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_TRUE(test_src_->Pause());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePaused);
  EXPECT_FALSE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePaused);
}

TEST_F(BaseSourceTest, TestStopFromStopped) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_FALSE(test_src_->Stop());
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
}

TEST_F(BaseSourceTest, TestStopFromPlaying) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_TRUE(test_src_->Stop());
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
}

TEST_F(BaseSourceTest, TestStopFromPaused) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_TRUE(test_src_->Pause());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePaused);
  EXPECT_TRUE(test_src_->Stop());
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
}

TEST_F(BaseSourceTest, TestPauseFromStopped) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_FALSE(test_src_->Pause());
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
}

TEST_F(BaseSourceTest, TestPauseFromPlaying) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_TRUE(test_src_->Pause());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePaused);
}

TEST_F(BaseSourceTest, TestPauseFromPaused) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_TRUE(test_src_->Pause());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePaused);
  EXPECT_FALSE(test_src_->Pause());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePaused);
}

TEST_F(BaseSourceTest, TestResumeFromStopped) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_FALSE(test_src_->Resume());
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
}

TEST_F(BaseSourceTest, TestResumeFromPlaying) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_FALSE(test_src_->Resume());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
}

TEST_F(BaseSourceTest, TestResumeFromPaused) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_TRUE(test_src_->Pause());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePaused);
  EXPECT_TRUE(test_src_->Resume());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
}

TEST_F(BaseSourceTest, TestStepFromStopped) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_FALSE(test_src_->Step());
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
}

TEST_F(BaseSourceTest, TestStepFromPlaying) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_FALSE(test_src_->Step());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
}

TEST_F(BaseSourceTest, TestStepFromPaused) {
  EXPECT_EQ(test_src_->GetState(), kStreamStateStopped);
  EXPECT_TRUE(test_src_->Start());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePlaying);
  EXPECT_TRUE(test_src_->Pause());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePaused);
  EXPECT_TRUE(test_src_->Step());
  EXPECT_EQ(test_src_->GetState(), kStreamStatePaused);
}