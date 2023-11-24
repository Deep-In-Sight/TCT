#pragma once
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

struct ImGuiContext;
struct ImGuiWidget;

struct Window {
  Window(const std::string& title, int width, int height, int x, int y,
         bool vsync = false);
  ~Window();

  void AddChild(std::shared_ptr<ImGuiWidget> child);
  void RemoveChild(std::shared_ptr<ImGuiWidget> child);
  bool Render();

  std::vector<std::shared_ptr<ImGuiWidget>> children_;
  GLFWwindow* glfwWindow_;
  ImGuiContext* imguiContext_;
};