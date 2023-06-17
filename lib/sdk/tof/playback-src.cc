#include <sdk/tof/playback-src.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace spdlog;

static logger *logger_ = stdout_color_mt("PlaybackSource").get();

PlaybackSource::PlaybackSource(const string &name, bool is_async, bool loop)
    : BaseSource(name, is_async),
      loop_(loop),
      file_(nullptr),
      fps_(30.0f),
      shape_({4, 480, 640}),
      type_(CV_16SC1) {}

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
  shape_ = shape;
  type_ = type;
  GetSourcePad()->SetFrameFormat(shape, type);
}

void PlaybackSource::SetFrameRate(float fps) {
  logger_->info("Setting playback source fps to {}", fps);
  fps_ = fps;
}

void PlaybackSource::SetLoop(bool loop) {
  logger_->info("Setting playback source loop to {}", loop);
  loop_ = loop;
}

Mat PlaybackSource::GenerateFrame() {
  Mat frame(shape_.dims(), shape_.p(), type_);
  float duration_ms = 1 / fps_ * 1000;

  std::this_thread::sleep_for(std::chrono::milliseconds((int)duration_ms));

  // logger_->info("Reading frame of shape: {} {}x{}x{} type: {}",
  // shape_.dims(),
  // shape_[0], shape_[1], shape_[2], type_);

  int read = fread(frame.data, 1, frame.total() * frame.elemSize(), file_);
  cout << read << "elem size" << frame.elemSize() << endl;

  if (read == frame.total() * frame.elemSize()) {
    logger_->info("Sending frame");
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