#include <sdk/core/element.h>
#include <sdk/core/pad.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;
static logger *logger_ = stdout_color_mt("Pad").get();

MatShape::MatShape() {
  dims_ = 1;
  p_[0] = 0;
}

MatShape::MatShape(int i0, int i1) {
  dims_ = 2;
  p_[0] = i0;
  p_[1] = i1;
}

MatShape::MatShape(int i0, int i1, int i2) {
  dims_ = 3;
  p_[0] = i0;
  p_[1] = i1;
  p_[2] = i2;
}

MatShape::MatShape(int i0, int i1, int i2, int i3) {
  dims_ = 4;
  p_[0] = i0;
  p_[1] = i1;
  p_[2] = i2;
  p_[3] = i3;
}

MatShape::MatShape(int i0, int i1, int i2, int i3, int i4) {
  dims_ = 5;
  p_[0] = i0;
  p_[1] = i1;
  p_[2] = i2;
  p_[3] = i3;
  p_[4] = i4;
}

MatShape::MatShape(int i0, int i1, int i2, int i3, int i4, int i5) {
  dims_ = 6;
  p_[0] = i0;
  p_[1] = i1;
  p_[2] = i2;
  p_[3] = i3;
  p_[4] = i4;
  p_[5] = i5;
}

MatShape::MatShape(initializer_list<int> sizes) {
  dims_ = sizes.size();
  assert(dims_ < sizeof(p_) / sizeof(int));
  for (int i = 0; i < dims_; i++) {
    p_[i] = *(sizes.begin() + i);
  }
}

MatShape::MatShape(const MatShape &shape) {
  dims_ = shape.dims_;
  for (int i = 0; i < dims_; i++) {
    p_[i] = shape.p_[i];
  }
}

MatShape::MatShape(const MatSize &size) {
  dims_ = size.dims();
  for (int i = 0; i < dims_; i++) {
    p_[i] = size[i];
  }
}

MatShape::~MatShape() {}

int MatShape::dims() const { return dims_; }
const int *MatShape::p() const { return p_; }

MatShape &MatShape::operator=(const MatShape &shape) {
  dims_ = shape.dims_;
  for (int i = 0; i < dims_; i++) {
    p_[i] = shape.p_[i];
  }
  return *this;
}

MatShape &MatShape::operator=(const MatSize &size) {
  dims_ = size.dims();
  for (int i = 0; i < dims_; i++) {
    p_[i] = size[i];
  }
  return *this;
}

bool MatShape::operator==(const MatShape &shape) {
  if (dims_ != shape.dims_) {
    return false;
  }
  for (int i = 0; i < dims_; i++) {
    if (p_[i] != shape.p_[i]) {
      return false;
    }
  }
  return true;
}

bool MatShape::operator!=(const MatShape &shape) {
  if (dims_ != shape.dims_) {
    return true;
  }
  for (int i = 0; i < dims_; i++) {
    if (p_[i] != shape.p_[i]) {
      return true;
    }
  }
  return false;
}

bool MatShape::operator==(const MatSize &size) {
  if (dims_ != size.dims()) {
    return false;
  }
  for (int i = 0; i < dims_; i++) {
    if (p_[i] != size[i]) {
      return false;
    }
  }
  return true;
}

bool MatShape::operator!=(const MatSize &size) {
  if (dims_ != size.dims()) {
    return true;
  }
  for (int i = 0; i < dims_; i++) {
    if (p_[i] != size[i]) {
      return true;
    }
  }
  return false;
}

const int MatShape::operator[](int index) const {
  assert(index < dims_);
  return p_[index];
}

Pad::Pad(PadDirection direction, const string &name = "")
    : mat_shape_(DEFAULT_MAT_SHAPE), mat_type_(DEFAULT_MAT_TYPE) {
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
  // update observers
  for (auto it = observers_.begin(); it != observers_.end(); it++) {
    (*it)->OnNewFrame(frame);
  }

  if (direction_ == kPadSource && link_status_ == kPadUnlinked) {
    return;
  }

  if (direction_ == kPadSink && parent_ == nullptr) {
    return;
  }

  // check size and type. frame.size, not frame.size(). frame.size() is the 2D
  // size of the matrix, in case dims=2. frame.size is the MatSize that can have
  // dims>2.
  if (mat_shape_ != frame.size || frame.type() != mat_type_) {
    logger_->error("Frame size or type does not match");
    return;
  }

  // send out frame
  if (direction_ == kPadSource) {
    peer_->PushFrame(frame);
  } else {
    parent_->PushFrame(frame);
  }
}

void Pad::AddObserver(PadObserver *observer) {
  if (observer == nullptr) {
    return;
  }
  for (auto it = observers_.begin(); it != observers_.end(); it++) {
    if (*it == observer) {
      return;
    }
  }
  observers_.push_back(observer);
  observer->SetFrameFormat(mat_shape_, mat_type_);
}

void Pad::RemoveObserver(PadObserver *observer) {
  if (observer == nullptr) {
    return;
  }
  observers_.remove(observer);
}

int Pad::GetObserverCount() { return observers_.size(); }

void Pad::SetFrameFormat(const MatShape &shape, int type) {
  mat_shape_ = shape;
  mat_type_ = type;

  for (auto it = observers_.begin(); it != observers_.end(); it++) {
    (*it)->SetFrameFormat(shape, type);
  }

  if (direction_ == kPadSource && link_status_ == kPadUnlinked) {
    return;
  }

  if (direction_ == kPadSink && parent_ == nullptr) {
    return;
  }

  if (direction_ == kPadSource) {
    peer_->SetFrameFormat(shape, type);
  } else {
    parent_->SetFrameFormat(shape, type);
  }
}

void Pad::GetFrameFormat(MatShape &shape, int &type) {
  shape = mat_shape_;
  type = mat_type_;
}

PadObserver::PadObserver()
    : mat_shape_(DEFAULT_MAT_SHAPE),
      mat_type_(DEFAULT_MAT_TYPE),
      channel_(kDepthChannel) {}

void PadObserver::SetFrameFormat(const MatShape &shape, int type) {
  if (mat_shape_.dims() != 3) {
    throw std::invalid_argument(
        "PadObserver only supports 3 dimension Mat (CHW)");
  }
  if (mat_shape_[0] != 1 && mat_shape_[0] != 2) {
    throw std::invalid_argument(
        "PadObserver only supports 1 or 2 channels for depth (and) amplitude");
  }
  if (mat_type_ != CV_32FC1) {
    throw std::invalid_argument("PadObserver only supports CV_32FC1 data type");
  }
  if (mat_shape_ != shape || mat_type_ != type) {
    mat_shape_ = shape;
    mat_type_ = type;
    OnFrameFormatChanged(shape, type);
  }
}

void PadObserver::SelectChannel(DepthAmplitudeChannel channel) {
  if ((channel != kDepthChannel) && (channel != kAmplitudeChannel)) {
    throw std::invalid_argument("Invalid channel");
  }
  if ((channel == kAmplitudeChannel) && (mat_shape_[0] == 1)) {
    throw std::invalid_argument("Pad only has depth channel");
  }
  channel_ = channel;
}