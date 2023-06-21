/* Copyright (C) 2023 Deep In Sight
 * Author: Le Ngoc Linh <lnlinh93@dinsight.ai>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
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