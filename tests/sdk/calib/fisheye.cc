// #include <sdk/calib/fisheye.h>

// class OpenCVSink : public BaseSink {
//  public:
//   OpenCVSink(const string& name = "") : BaseSink(name) {}
//   void SinkFrame(Mat& frame) override {
//     cv::imshow("OpenCVSink", frame);
//     cv::waitKey();
//   }
// };

// TEST(FisheyeTest, TestTransformFrame) {
//   Fisheye fisheye;
//   fisheye.SetParams({390.33216865, 392.37363314,  // fx, fy
//                      337.7728288, 244.44454662,   // cx, cy
//                      -0.34335484, 0.14217051,     // k1, k2
//                      0.00073704, -0.00061519,     // p1, p2
//                      -0.03051488});               // p3
//   OpenCVSink sink;
//   fisheye.GetSourcePad()->Link(sink.GetSinkPad());
// }