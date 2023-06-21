#include <arpa/inet.h>
#include <sdk/tof/camera-src.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace spdlog;

static logger* logger_ = stdout_color_mt("ToFCameraSrc").get();

IPCamera::IPCamera() { socketFd_ = -1; }

IPCamera::~IPCamera() { Close(); }

bool IPCamera::Open(string host, int port) {
  logger_->info("Open: host={}, port={}", host, port);

  socketFd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFd_ < 0) {
    logger_->error("Failed to create socket");
    return false;
  }

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr(host.c_str());

  if (connect(socketFd_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) <
      0) {
    logger_->error("Failed to connect to camera");
    return false;
  }

  return true;
}

bool IPCamera::Close() {
  if (socketFd_ < 0) {
    return false;
  }

  close(socketFd_);
  socketFd_ = -1;

  return true;
}

int IPCamera::Read(char* buffer, int size) {
  if (socketFd_ < 0) {
    return -1;
  }

  int remain = size;
  while (remain > 0) {
    int n = read(socketFd_, buffer, remain);
    if (n < 0) {
      return -1;
    }

    remain -= n;
    buffer += n;
  }

  return size;
}

int IPCamera::Write(const char* buffer, int size) {
  if (socketFd_ < 0) {
    return -1;
  }

  int remain = size;
  while (remain > 0) {
    int n = write(socketFd_, buffer, remain);
    if (n < 0) {
      return -1;
    }

    remain -= n;
    buffer += n;
  }

  return size;
}

ToFCameraSrc::ToFCameraSrc(const string& name, CameraType type)
    : BaseSource(name, true) {
  name_ = name;
  switch (type) {
    case kIPCamera:
      cameraImpl_ = new IPCamera();
      break;
    default:
      cameraImpl_ = nullptr;
      break;
  }
}

ToFCameraSrc::~ToFCameraSrc() { delete cameraImpl_; }

void ToFCameraSrc::SetDeviceName(const string& name) { name_ = name; }

bool ToFCameraSrc::InitializeSource() {
  // split name into host and port
  size_t pos = name_.find(":");
  string address;
  int port;
  if (pos != string::npos) {
    address = name_.substr(0, pos);
    port = stoi(name_.substr(pos + 1));
  } else {
    throw std::runtime_error("Invalid camera device name");
  }

  if (!cameraImpl_->Open(address, port)) {
    throw std::runtime_error("Failed to open camera");
  }

  InitSensor();

  shape_ = MatShape({4, 480, 640});
  type_ = CV_16SC1;
  GetSourcePad()->SetFrameFormat(shape_, type_);

  return true;
};

Mat ToFCameraSrc::GenerateFrame() {
  Mat frame({shape_[0], shape_[1], shape_[2]}, type_);
  string cmd = "getFrame\0";
  cameraImpl_->Write(cmd.c_str(), cmd.size());
  cameraImpl_->Read((char*)frame.data, frame.total() * frame.elemSize());
  return frame;
}

void ToFCameraSrc::CleanupSource() { cameraImpl_->Close(); }

void ToFCameraSrc::InitSensor() {
  string cmd;
  int16_t ack;

  // set modulation frequency (commedted out because camera is locked to 20MHz)
  // cmd = "changeModFreq 20\0";
  // cameraImpl_->Write(cmd.c_str(), cmd.size());
  // cameraImpl_->Read((char*)&ack, sizeof(ack));
  // if (ack != 0) {
  //   logger_->error("Failed to set modulation frequency");
  //   return;
  // }

  // disable range check
  cmd = "w 0x1433 0x00\0";
  cameraImpl_->Write(cmd.c_str(), cmd.size());
  cameraImpl_->Read((char*)&ack, sizeof(ack));

  // set fpga mode
  cmd = "setMode 0\0";
  cameraImpl_->Write(cmd.c_str(), cmd.size());
  cameraImpl_->Read((char*)&ack, sizeof(ack));

  // set output mode
  cmd = "changeOutputMode 0\0";
  cameraImpl_->Write(cmd.c_str(), cmd.size());
  cameraImpl_->Read((char*)&ack, sizeof(ack));

  // set phase delays
  cmd = "w 0x217c 0x0c\0";
  cameraImpl_->Write(cmd.c_str(), cmd.size());
  cameraImpl_->Read((char*)&ack, sizeof(ack));

  cmd = "w 0x2184 0x0d\0";
  cameraImpl_->Write(cmd.c_str(), cmd.size());
  cameraImpl_->Read((char*)&ack, sizeof(ack));

  cmd = "w 0x2188 0x0d\0";
  cameraImpl_->Write(cmd.c_str(), cmd.size());
  cameraImpl_->Read((char*)&ack, sizeof(ack));

  cmd = "w 0x2189 0x13\0";
  cameraImpl_->Write(cmd.c_str(), cmd.size());
  cameraImpl_->Read((char*)&ack, sizeof(ack));

  cmd = "w 0x218a 0x18\0";
  cameraImpl_->Write(cmd.c_str(), cmd.size());
  cameraImpl_->Read((char*)&ack, sizeof(ack));

  cmd = "w 0x218b 0x1F\0";
  cameraImpl_->Write(cmd.c_str(), cmd.size());
  cameraImpl_->Read((char*)&ack, sizeof(ack));
}
bool ToFCameraSrc::SetFmod(int fmodMHz) { return true; }

bool ToFCameraSrc::SetMode(string mode = "raw") { return true; }