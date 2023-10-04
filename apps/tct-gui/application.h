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

struct Application {
  static Application& GetInstance();
  ~Application();

  void Create();
  void Run();
  const nlohmann::json& GetConfig();
  GLuint GetTexture(const std::string& name);

 private:
  Application();
  void LoadResources();
  void CreateFontAtlas();
  void CreateTextures();

  GLFWwindow* glfwWindow;
  ImGuiContext* imguiContext;

  nlohmann::json appConfig;
  std::unordered_map<std::string, GLuint> textures;

  std::vector<std::shared_ptr<ImGuiWidget>> children;
};

void LoadIcon(const char* filename, GLuint* textureId, ImVec2* size);