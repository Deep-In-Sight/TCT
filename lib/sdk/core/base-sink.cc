#include <sdk/core/base-sink.h>
#include <sdk/core/pad.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;

static logger *logger_ = stdout_color_mt("BaseSink").get();

BaseSink::BaseSink(const string &name) : Element(name) {
  sink_pad_ = new Pad(kPadSink, "sink");
  AddPad(sink_pad_);
}

BaseSink::~BaseSink() {
  sink_pad_->Unlink();
  delete sink_pad_;
}

void BaseSink::PushFrame(Mat &frame) { SinkFrame(frame); }

Pad *BaseSink::GetSinkPad() { return sink_pad_; }
