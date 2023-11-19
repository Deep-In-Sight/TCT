#include "utility.h"

#include <GL/gl.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

void UploadCvMatToGpuTexture(const cv::Mat& image, GLuint* pTextureId,
                             ImVec2* pSize) {
  // convert image to RGBA
  cv::Mat rgba;
  auto convertCode = cv::COLOR_BGR2RGBA;
  switch (image.type()) {
    case CV_8UC1:
      convertCode = cv::COLOR_GRAY2RGBA;
      break;
    case CV_8UC3:
      convertCode = cv::COLOR_BGR2RGBA;
      break;
    case CV_8UC4:
      convertCode = cv::COLOR_BGRA2RGBA;
      break;
    default:
      convertCode = cv::COLOR_BGR2RGBA;
      break;
  }

  cv::cvtColor(image, rgba, cv::COLOR_BGR2RGBA);

  // create texture
  if (*pTextureId == 0) {
    glGenTextures(1, pTextureId);
  }

  glBindTexture(GL_TEXTURE_2D, *pTextureId);
  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // This is required on WebGL for non power-of-two textures
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba.cols, rgba.rows, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, rgba.data);

  // unbind texture
  glBindTexture(GL_TEXTURE_2D, 0);

  // set image size
  if (pSize != nullptr) *pSize = ImVec2(rgba.cols, rgba.rows);
}

std::vector<cv::Mat> splitChannels(const cv::Mat& image) {
  std::vector<cv::Mat> channels;
  int channel, width, height;
  auto size = image.size;  // not image.size()

  channel = size.dims() > 2 ? size[0] : 1;
  height = size.dims() > 2 ? size[1] : size[0];
  width = size.dims() > 2 ? size[2] : size[1];

  for (int c = 0; c < channel; c++) {
    uint8_t* data = image.data + c * width * height * image.elemSize();
    channels.push_back(cv::Mat(height, width, image.type(), data));
  }

  return channels;
}