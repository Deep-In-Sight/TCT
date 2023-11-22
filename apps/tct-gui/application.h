#pragma once
#include <GLFW/glfw3.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "imgui-widget.h"
#include "nlohmann/json.hpp"

struct NodeEditor;
struct DepthInspector;
struct ImageDisplay;
struct GLFWwindow;
struct ImGuiContext;
struct ImGuiGLWFWindow;

struct Application {
  static Application& GetInstance();
  ~Application();

  void Create();
  void Run();
  const nlohmann::json& GetConfig();

 private:
  Application();

  nlohmann::json appConfig;
  std::unordered_map<std::string, GLuint> textures;

  std::vector<std::shared_ptr<ImGuiGLWFWindow>> children;
};