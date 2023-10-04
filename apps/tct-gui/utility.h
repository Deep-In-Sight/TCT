#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <opencv2/core.hpp>

void UploadCvMatToGpuTexture(const cv::Mat& image, GLuint* textureId,
                             ImVec2* size = nullptr);