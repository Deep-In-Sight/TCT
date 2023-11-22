#pragma once
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <memory>
#include <vector>

struct ImGuiWidget;
struct ImGuiGLWFWindow {
  ImGuiGLWFWindow(const std::string& title, int width, int height, int x,
                  int y);
  ~ImGuiGLWFWindow();

  void AddChild(std::shared_ptr<ImGuiWidget> child);
  void RemoveChild(std::shared_ptr<ImGuiWidget> child);
  bool RenderWindow();

  std::vector<std::shared_ptr<ImGuiWidget>> children_;
  GLFWwindow* glfwWindow_;
  ImGuiContext* imguiContext_;
};