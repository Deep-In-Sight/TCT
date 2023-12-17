#include "application.h"

#include <GLFW/glfw3.h>

#include <fstream>

#include "inspector-bitmap-view.h"
#include "node-editor.h"
#include "sdk/tof/depth-calc.h"
#include "sdk/tof/moving-average.h"
#include "sdk/tof/playback-src.h"
#include "utility.h"
#include "window.h"

Application& Application::GetInstance() {
  static Application instance;
  return instance;
}

Application::Application() {
  // Load config file
  std::ifstream configFile("data/appconfig.json");
  appConfig = nlohmann::json::parse(configFile);

  BackEnd::Initialize(BACKEND_SDL2);
}

Application::~Application() { BackEnd::Destroy(); }

void Application::Create() {
  // Create main window
  auto mainWindowConfig = appConfig["layout"]["mainWindow"];
  auto title = mainWindowConfig["title"].get<std::string>();
  auto width = mainWindowConfig["width"].get<int>();
  auto height = mainWindowConfig["height"].get<int>();
  auto x = mainWindowConfig["x"].get<int>();
  auto y = mainWindowConfig["y"].get<int>();

  auto mainWindow =
      BackEnd::CreateWindow(title, x, y, width, height, WindowFlags_VSYNC);
  auto nodeEditor = std::make_shared<NodeEditor>();
  mainWindow->AddChild(nodeEditor);

  AddWindow(mainWindow);
}

void Application::AddWindow(std::shared_ptr<Window> window) {
  children.push_back(window);
}

void Application::RemoveWindow(std::shared_ptr<Window> window) {
  children.remove(window);
}

std::shared_ptr<Window> Application::GetWindow(const std::string& title) {
  for (auto window : children) {
    if (window->title_ == title) {
      return window;
    }
  }
  return nullptr;
}

void Application::Run() {
  bool shouldClose = false;
  while (!shouldClose) {
    BackEnd::ProcessEvents();

    for (auto window = children.begin(); window != children.end(); ++window) {
      shouldClose |= (*window)->Render();
    }
  }
}

const nlohmann::json& Application::GetConfig() { return appConfig; }