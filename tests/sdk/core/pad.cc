#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/core/element.h>
#include <sdk/core/pad.h>
// #include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

class ElementMock : public Element {
 public:
  ElementMock() {}
  ~ElementMock() {}
  MOCK_METHOD(void, PushFrame, (cv::Mat & frame), (override));
};

TEST(PadTest, TestPadContructor) {
  PadDirection direction = kPadSource;
  string name = "src";
  Pad pad(direction, name);
  EXPECT_TRUE(pad.GetName() == name);
  EXPECT_EQ(pad.GetDirection(), kPadSource);
  EXPECT_EQ(pad.GetLinkStatus(), kPadUnlinked);
  EXPECT_EQ(pad.GetParent(), nullptr);
}

TEST(PadTest, TestPadSetParent) {
  Pad pad(kPadSink, "sink");
  Element elem1;
  EXPECT_EQ(pad.SetParent(&elem1), true);
  Element elem2;
  EXPECT_EQ(pad.SetParent(&elem2), false);
}

TEST(PadTest, TestPadLink) {
  Pad pad1(kPadSource, "src");
  Pad pad2(kPadSink, "sink");
  EXPECT_EQ(pad1.Link(&pad2), kPadLinkOk);
  EXPECT_EQ(pad1.GetLinkStatus(), kPadLinked);
  EXPECT_EQ(pad2.GetLinkStatus(), kPadLinked);
  EXPECT_EQ(pad1.GetPeer(), &pad2);
  EXPECT_EQ(pad2.GetPeer(), &pad1);
  Pad pad3(kPadSink, "sink");
  EXPECT_EQ(pad1.Link(&pad3), kPadLinkAlreadyLinked);
  Pad pad4(kPadSink, "sink");
  Pad pad5(kPadSink, "sink");
  EXPECT_EQ(pad4.Link(&pad4), kPadLinkWrongDirection);

  Pad pad6(kPadSource, "src");
  Pad pad7(kPadSource, "src");
  EXPECT_EQ(pad6.Link(&pad7), kPadLinkWrongDirection);
}

TEST(PadTest, TestPadUnlink) {
  Pad pad1(kPadSource, "src");
  Pad pad2(kPadSink, "sink");
  pad1.Link(&pad2);
  EXPECT_EQ(pad1.Unlink(), kPadLinkOk);
  EXPECT_EQ(pad1.GetLinkStatus(), kPadUnlinked);
  EXPECT_EQ(pad2.GetLinkStatus(), kPadUnlinked);
  EXPECT_EQ(pad1.GetPeer(), nullptr);
  EXPECT_EQ(pad2.GetPeer(), nullptr);
  EXPECT_EQ(pad1.Unlink(), kPadLinkOk);
}

TEST(PadTest, TestPadPushFrame) {
  Pad pad1(kPadSource, "src");
  Pad pad2(kPadSink, "sink");
  ElementMock elem;
  ElementMock elem2;

  EXPECT_CALL(elem, PushFrame).Times(1);
  EXPECT_CALL(elem2, PushFrame).Times(1);

  cv::Mat frame;

  pad1.Link(&pad2);
  pad2.SetParent(&elem);
  pad1.PushFrame(frame);

  Pad pad3(kPadSink, "sink");
  pad3.SetParent(&elem2);
  pad3.PushFrame(frame);
}