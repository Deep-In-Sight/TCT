#include "element.h"

#include <sdk/core/element.h>
#include <sdk/core/pad.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;
static logger *logger_ = stdout_color_mt("Element").get();

Element::Element(const string &name) : name_(name) {}

Element::~Element() {}

const string &Element::GetName() { return name_; }

void Element::SetName(const string &name) { name_ = name; }

Pad *Element::GetPad(const string &name) {
  for (auto it = pads_.begin(); it != pads_.end(); it++) {
    if ((*it)->GetName() == name) {
      return *it;
    }
  }
  return nullptr;
}

bool Element::AddPad(Pad *pad) {
  for (auto it = pads_.begin(); it != pads_.end(); it++) {
    if ((*it)->GetName() == pad->GetName()) {
      logger_->error("Pad {} already exists", pad->GetName());
      return false;
    }
  }
  pads_.push_back(pad);
  return pad->SetParent(this);
}

void Element::SetFrameFormat(const MatShape &shape, int type) {
  for (auto it = pads_.begin(); it != pads_.end(); it++) {
    if ((*it)->GetDirection() == kPadSource) {
      (*it)->SetFrameFormat(shape, type);
    }
  }
}
