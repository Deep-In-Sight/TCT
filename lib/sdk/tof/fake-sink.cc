#include <sdk/tof/fake-sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;

static logger *logger_ = stdout_color_mt("FakeSink").get();

FakeSink::FakeSink(const string &name) : BaseSink(name) {
  current_ = std::chrono::system_clock::now();
}

FakeSink::~FakeSink() {}

void FakeSink::SinkFrame(Mat &frame) {
  logger_->info("sinking frame");
  static int frame_cnt = 0;
  if (frame_cnt++ % 100 == 0) {
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = now - current_;
    current_ = now;
    logger_->info("FakeSink received frame at {} fps",
                  100 / elapsed_seconds.count());
  }
}