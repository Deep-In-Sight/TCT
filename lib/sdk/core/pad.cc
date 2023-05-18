#include <sdk/core/element.h>
#include <sdk/core/pad.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;
static logger *logger_ = stdout_color_mt("Pad").get();

Pad::Pad(PadDirection direction, const string &name = "") {
  direction_ = direction;
  link_status_ = kPadUnlinked;
  parent_ = nullptr;
  peer_ = nullptr;
  name_ = name;
  if (name_.empty()) {
    name_ = (direction == kPadSource) ? "src" : "sink";
  }
}

Pad::~Pad() {}

void Pad::SetName(const string &name) { name_ = name; }

const string &Pad::GetName() { return name_; }

PadDirection Pad::GetDirection() { return direction_; }

PadLinkStatus Pad::GetLinkStatus() { return link_status_; }

bool Pad::SetParent(Element *parent) {
  if (parent_ != nullptr && parent_ != parent) {
    return false;
  }
  parent_ = parent;
  return true;
}

Element *Pad::GetParent() { return parent_; }

PadLinkReturn Pad::Link(Pad *peer) {
  if (peer_ != nullptr && peer_ != peer) {
    return kPadLinkAlreadyLinked;
  }

  if (direction_ == peer->GetDirection()) {
    return kPadLinkWrongDirection;
  }

  if (peer_ == peer) {
    return kPadLinkOk;
  }

  peer_ = peer;
  PadLinkReturn ret = peer_->Link(this);

  if (ret != kPadLinkOk) {
    peer_ = nullptr;
    link_status_ = kPadUnlinked;
    return ret;
  } else {
    link_status_ = kPadLinked;
    return kPadLinkOk;
  }
}

PadLinkReturn Pad::Unlink() {
  if (link_status_ == kPadUnlinked) {
    return kPadLinkOk;
  }

  link_status_ = kPadUnlinked;
  peer_->Unlink();
  peer_ = nullptr;

  return kPadLinkOk;
}

Pad *Pad::GetPeer() { return peer_; }

void Pad::PushFrame(cv::Mat &frame) {
  if (direction_ == kPadSource && link_status_ == kPadUnlinked) {
    return;
  }

  if (direction_ == kPadSink && parent_ == nullptr) {
    return;
  }

  // run probes

  // send out frame
  if (direction_ == kPadSource) {
    peer_->PushFrame(frame);
  } else {
    parent_->PushFrame(frame);
  }
}
