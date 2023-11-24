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

  // Initialize GLFW
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
}

Application::~Application() { glfwTerminate(); }

void Application::Create() {
  // Create main window
  auto mainWindowConfig = appConfig["layout"]["mainWindow"];
  auto title = mainWindowConfig["title"].get<std::string>();
  auto width = mainWindowConfig["width"].get<int>();
  auto height = mainWindowConfig["height"].get<int>();
  auto x = mainWindowConfig["x"].get<int>();
  auto y = mainWindowConfig["y"].get<int>();

  auto mainWindow = std::make_shared<Window>(title, width, height, x, y, true);
  auto nodeEditor = std::make_shared<NodeEditor>();
  mainWindow->AddChild(nodeEditor);

  children.push_back(mainWindow);
}

void Application::Run() {
  bool shouldClose = false;

  // pipeline construct
  auto src = new PlaybackSource("filesrc", false, false);
  auto depthCalc = new DepthCalc("raw2depth");
  auto ma = new MovingAverage("movingAverage");
  auto inspector = new InspectorBitmapView();
  src->GetSourcePad()->Link(depthCalc->GetSinkPad());
  depthCalc->GetSourcePad()->Link(ma->GetSinkPad());
  ma->GetSourcePad()->AddObserver(inspector);

  // pipeline config
  src->SetFilename("./data/videos/37MHz_1.4m_73x4x480x640_16SC1.bin");
  src->SetLoop(true);
  src->SetFrameRate(30);
  src->SetFormat(MatShape(4, 480, 640), CV_16SC1);
  int fmodMHz = 37;
  float offset = 1.4;
  depthCalc->SetConfig(fmodMHz * 1e6, offset);
  int MAwidth = 32;
  ma->SetWindowSize(MAwidth);

  auto inspectorWindow = std::make_shared<Window>("Inspector", 1280, 600, 0, 0);
  inspectorWindow->AddChild(std::shared_ptr<ImGuiWidget>(inspector));

  children.push_back(inspectorWindow);

  // start pipeline
  src->Start();

  while (!shouldClose) {
    glfwPollEvents();

    for (auto window : children) {
      shouldClose |= window->Render();
    }
  }
}

const nlohmann::json& Application::GetConfig() { return appConfig; }