#ifndef __TOF_CAMERA_H__
#define __TOF_CAMERA_H__

#include <sdk/core/base-src.h>

enum CameraType {
  kIPCamera,
  // kV4L2Camera,
  // kUVCCamera,
  kUnknownCamera
};

class IPCamera {
 public:
  IPCamera();
  ~IPCamera();
  bool Open(string host, int port);
  bool Close();
  int Read(char* buffer, int size);
  int Write(const char* buffer, int size);

 private:
  int socketFd_;
};

class ToFCameraSrc : public BaseSource {
 public:
  /**
   * @brief Create new camera object
   *
   * @param name for IP camera, name should be in the format of "host:port"
   * @param type
   */
  ToFCameraSrc(const string& name, CameraType type = kIPCamera);
  ~ToFCameraSrc();

  void SetDeviceName(const string& name);

  bool InitializeSource() override;
  Mat GenerateFrame() override;
  void CleanupSource() override;

  bool SetFmod(int fmodMHz);
  bool SetMode(string mode);

  void InitSensor();

 private:
  string deviceName_;
  IPCamera* cameraImpl_;
  string name_;
  MatShape shape_;
  int type_;
};

#endif  // __TOF_CAMERA_H__