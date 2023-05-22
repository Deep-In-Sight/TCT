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

void BaseSource::GenerateLoop() {
  while (running_) {
    cv::Mat frame = GenerateFrame();
    source_pad_->PushFrame(frame);
  }
}

Pad *BaseSource::GetSourcePad() { return source_pad_; }

void BaseSource::Start() {
  running_ = true;
  thread_ = new thread(&BaseSource::GenerateLoop, this);
}

void BaseSource::Stop() {
  running_ = false;
  thread_->join();
  delete thread_;
}

bool BaseSource::IsRunning() { return running_; }