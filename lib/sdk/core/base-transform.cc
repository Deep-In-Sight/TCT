#include <sdk/core/base-transform.h>
#include <sdk/core/pad.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;

static logger *logger_ = stdout_color_mt("BaseTransform").get();

BaseTransform::BaseTransform(const string &name) : Element(name) {
  source_pad_ = new Pad(kPadSource, "src");
  sink_pad_ = new Pad(kPadSink, "sink");
  AddPad(source_pad_);
  AddPad(sink_pad_);
}

BaseTransform::~BaseTransform() {
  source_pad_->Unlink();
  sink_pad_->Unlink();
  delete source_pad_;
  delete sink_pad_;
}

void BaseTransform::PushFrame(Mat &frame) { TransformFrame(frame); }

void BaseTransform::TransformFrame(Mat &frame) {
  source_pad_->PushFrame(frame);
}

void BaseTransform::PushState(StreamState state) {
  source_pad_->PushState(state);
}

Pad *BaseTransform::GetSinkPad() { return sink_pad_; }

Pad *BaseTransform::GetSourcePad() { return source_pad_; }