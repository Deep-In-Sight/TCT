#include <sdk/core/base-src.h>
#include <sdk/core/pad.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;

static logger *logger_ = stdout_color_mt("BaseSource").get();

BaseSource::BaseSource(const string &name) : Element(name) {
  source_pad_ = new Pad(kPadSource, "src");
  AddPad(source_pad_);
  running_ = false;
}

BaseSource::~BaseSource() { delete source_pad_; }

void BaseSource::PushFrame(Mat &frame) {}

void BaseSource::PushFrame() {
  while (running_) {
    if (source_pad_->GetLinkStatus() == kPadLinked) {
      cv::Mat frame = GenerateFrame();
      PushFrame(frame);
    }
  }
}

Pad *BaseSource::GetSourcePad() { return source_pad_; }

void BaseSource::Start() { running_ = true; }

void BaseSource::Stop() { running_ = false; }