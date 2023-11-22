#include "utility.h"

#include <GL/gl.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "application.h"

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

ScrollingBuffer::ScrollingBuffer(int max_size) {
  MaxSize = max_size;
  Offset = 0;
  Total = 0;
  Data.reserve(MaxSize);
}

void ScrollingBuffer::AddPoint(float x, float y) {
  if (Data.size() < MaxSize)
    Data.push_back(ImVec2(x, y));
  else {
    Data[Offset] = ImVec2(x, y);
    Offset = (Offset + 1) % MaxSize;
  }
  Total++;
}

void ScrollingBuffer::Erase() {
  if (Data.size() > 0) {
    Data.shrink(0);
    Offset = 0;
  }
}
bool ScrollingBuffer::IsEmpty() { return Data.size() == 0; }

void CreateFontAtlas() {
  ImGuiIO& io = ImGui::GetIO();

  IM_DELETE(io.Fonts);

  io.Fonts = IM_NEW(ImFontAtlas);

  auto& app = Application::GetInstance();
  auto& appConfig = app.GetConfig();

  ImFontConfig fontConfig;
  auto fontConfigJson = appConfig["resources"]["fonts"]["mainFont"];
  fontConfig.SizePixels = fontConfigJson["config"]["sizePixels"].get<float>();
  fontConfig.OversampleH = fontConfigJson["config"]["oversampleH"].get<int>();
  fontConfig.OversampleV = fontConfigJson["config"]["oversampleV"].get<int>();
  fontConfig.PixelSnapH = fontConfigJson["config"]["pixelSnapH"].get<bool>();

  auto fontFile = fontConfigJson["ttf"].get<std::string>();

  io.Fonts->AddFontFromFileTTF(fontFile.c_str(), fontConfig.SizePixels,
                               &fontConfig);
  io.Fonts->Build();
}

void HelpMarker(const char* desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) &&
      ImGui::BeginTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}