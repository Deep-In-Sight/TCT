#include <sdk/core/base-sink.h>

#include <chrono>

class FakeSink : public BaseSink {
 public:
  FakeSink(const string& name = "");
  ~FakeSink();

 private:
  void SinkFrame(Mat& frame) override;
  // a variable to store current time
  std::chrono::time_point<std::chrono::system_clock> current_;
};