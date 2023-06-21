#include <sdk/tof/playback-src.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace spdlog;

static logger *logger_ = stdout_color_mt("PlaybackSource").get();

PlaybackSource::PlaybackSource(const string &name, bool is_async, bool loop)
    : BaseSource(name, is_async),
      loop_(loop),
      file_(nullptr),
      frame_duration_(1.0f / 30.0f),
      shape_({4, 480, 640}),
      type_(CV_16SC1) {
  last_frame_time_ = chrono::steady_clock::now();
}

PlaybackSource::~PlaybackSource() {
  if (GetState() != kStreamStateStopped) {
    Stop();
  }
}

void PlaybackSource::SetFilename(const string &filename) {
  logger_->info("Setting playback source filename to {}", filename);
  filename_ = filename;
}

bool PlaybackSource::InitializeSource() {
  logger_->info("Initializing playback source");

  file_ = fopen(filename_.c_str(), "rb");
  if (file_ == nullptr) {
    logger_->error("Failed to open file {}", filename_);
    return false;
  }

  if (shape_.dims() != 3) {
    logger_->error("Did you forget to set format?");
    return false;
  }

  // check file size
  fseek(file_, 0, SEEK_END);
  long size = ftell(file_);
  fseek(file_, 0, SEEK_SET);
  logger_->info("File size: {} bytes", size);

  // Disable next element format check. Temporary workaround. Until we have a
  // real n-dim matrix class to replace OpenCV Mat.
  // GetSourcePad()->SetFrameFormat({4, 480, 640}, CV_16SC1);
  return true;
}

void PlaybackSource::CleanupSource() {
  if (file_ != nullptr) {
    logger_->info("Cleaning up playback source");
    fclose(file_);
    file_ = nullptr;
  }
}

void PlaybackSource::SetFormat(const MatShape &shape, int type) {
  logger_->info("Setting playback source format to {}x{}x{} type: {}", shape[0],
                shape[1], shape[2], type);
  shape_ = shape;
  type_ = type;
  GetSourcePad()->SetFrameFormat(shape, type);
}

void PlaybackSource::SetFrameRate(float fps) {
  logger_->info("Setting playback source fps to {}", fps);
  frame_duration_ = 1.0 / fps;
  sleep_duration_ms_ = frame_duration_ * 1000;
}

void PlaybackSource::SetLoop(bool loop) {
  logger_->info("Setting playback source loop to {}", loop);
  loop_ = loop;
}

Mat PlaybackSource::GenerateFrame() {
  Mat frame(shape_.dims(), shape_.p(), type_);

  auto elapsed = (chrono::steady_clock::now() - last_frame_time_);
  last_frame_time_ = chrono::steady_clock::now();
  float elapsed_ms =
      chrono::duration_cast<chrono::milliseconds>(elapsed).count();
  float frame_duration_ms = frame_duration_ * 1000.0f;
  float sleep_adjust_ms = elapsed_ms - frame_duration_ms;
  float sleep_ms = sleep_duration_ms_ - sleep_adjust_ms;
  if (sleep_ms <= 0) {
    sleep_ms = 0;
  } else {
    std::this_thread::sleep_for(std::chrono::milliseconds((int)sleep_ms));
  }
  sleep_duration_ms_ = sleep_ms;

  // logger_->info("Reading frame of shape: {} {}x{}x{} type: {}",
  // shape_.dims(),
  // shape_[0], shape_[1], shape_[2], type_);

  int read = fread(frame.data, 1, frame.total() * frame.elemSize(), file_);

  if (read == frame.total() * frame.elemSize()) {
    // logger_->info("Sending frame");
    return frame;
  } else if (loop_) {
    logger_->info("Reached end of file, looping");
    fseek(file_, 0, SEEK_SET);
    fread(frame.data, frame.elemSize(), frame.total(), file_);
    return frame;
  } else {
    logger_->info("Reached end of file, stopping");
    return Mat();
  }
}