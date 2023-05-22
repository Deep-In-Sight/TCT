#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/core/base-transform.h>
#include <sdk/core/element.h>
#include <sdk/core/pad.h>
// #include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

class ElementMock : public Element {
 public:
  ElementMock(const string& name = "") : Element(name) {}
  ~ElementMock() {}
  MOCK_METHOD(void, PushFrame, (cv::Mat & frame), (override));
};

class TransformMock : public BaseTransform {
 public:
  TransformMock(const string& name = "") : BaseTransform(name) {}
  ~TransformMock() {}
  MOCK_METHOD(void, TransformFrame, (cv::Mat & frame), (override));
};

class PadObserverMock : public PadObserver {
 public:
  PadObserverMock() {}
  ~PadObserverMock() {}
  MOCK_METHOD(void, OnNewFrame, (cv::Mat & frame), (override));
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
  ElementMock elem1;
  EXPECT_EQ(pad.SetParent(&elem1), true);
  ElementMock elem2;
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

  cv::Mat frame(DEFAULT_MAT_SIZE, DEFAULT_MAT_TYPE);

  pad1.Link(&pad2);
  pad2.SetParent(&elem);
  pad1.PushFrame(frame);

  Pad pad3(kPadSink, "sink");
  pad3.SetParent(&elem2);
  pad3.PushFrame(frame);
}

TEST(PadTest, TestPadObservers) {
  Pad pad(kPadSource, "src");
  PadObserverMock observer1;
  PadObserverMock observer2;
  pad.AddObserver(&observer1);
  pad.AddObserver(&observer2);
  cv::Mat frame(Size(10, 10), CV_32FC1);
  EXPECT_CALL(observer1, OnNewFrame).Times(2);
  EXPECT_CALL(observer2, OnNewFrame).Times(1);
  pad.PushFrame(frame);
  pad.RemoveObserver(&observer2);
  pad.PushFrame(frame);
}

TEST(PadTest, TestSetSizeTypeChain) {
  string name = "elem";
  TransformMock elem(name);
  Pad* source_pad = elem.GetSourcePad();
  Pad* sink_pad = elem.GetSinkPad();

  Pad p(kPadSource, "src0");

  p.Link(sink_pad);
  p.SetSizeType(Size(100, 100), CV_8UC3);

  Size size;
  int type;
  source_pad->GetSizeType(size, type);

  // check src0->source_pad->elem->sink_pad propagation
  EXPECT_EQ(size.width, 100);
  EXPECT_EQ(size.height, 100);
  EXPECT_EQ(type, CV_8UC3);
}

TEST(PadTest, TestPadObserverSetChannel) {
  Pad p(kPadSource, "src");
  PadObserverMock observer;
  p.AddObserver(&observer);
  p.SetSizeType(Size(100, 100), CV_32FC2);
  EXPECT_NO_THROW(observer.SelectChannel(kDepthChannel));
  EXPECT_NO_THROW(observer.SelectChannel(kAmplitudeChannel));
  p.SetSizeType(Size(100, 100), CV_32FC1);
  EXPECT_NO_THROW(observer.SelectChannel(kDepthChannel));
  EXPECT_ANY_THROW(observer.SelectChannel(kAmplitudeChannel));
}

TEST(TestOpenCV, TestAt) {
  float data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  cv::Mat mat(2, 2, CV_32FC2, data);
  EXPECT_EQ(mat.at<cv::Vec2f>(0, 0)[0], 1);
  EXPECT_EQ(mat.at<cv::Vec2f>(0, 0)[1], 2);

  EXPECT_NE(CV_MAT_CN(CV_8UC1), 3);
  EXPECT_EQ(CV_MAT_CN(CV_8UC1), 1);
  EXPECT_EQ(CV_MAT_CN(CV_8UC2), 2);
  EXPECT_EQ(CV_MAT_CN(CV_8UC3), 3);
  std::cout << "CV_32FC1 = " << CV_32FC1 << std::endl;
  std::cout << "CV_32FC2 = " << CV_32FC2 << std::endl;
  std::cout << "CV_32FC3 = " << CV_32FC3 << std::endl;
}