#include "application.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <thread>

#include "image-widget.h"
#include "inspector-2d.h"
#include "node-editor.h"

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

  // Create a window
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // no exception
  auto title = appConfig["layout"]["mainWindow"]["title"].get<std::string>();
  auto w = appConfig["layout"]["mainWindow"]["width"].get<int>();
  auto h = appConfig["layout"]["mainWindow"]["height"].get<int>();
  auto x = appConfig["layout"]["mainWindow"]["x"].get<int>();
  auto y = appConfig["layout"]["mainWindow"]["y"].get<int>();

  glfwWindow = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
  if (!glfwWindow) {
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwSetWindowPos(glfwWindow, x, y);
  glfwMakeContextCurrent(glfwWindow);

  // Initialize ImGui
  IMGUI_CHECKVERSION();

  imguiContext = ImGui::CreateContext();
  ImGui::SetCurrentContext(imguiContext);

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  LoadResources();
}

Application::~Application() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(glfwWindow);
  glfwTerminate();
}

void Application::Create() {
  auto nodeEditor = std::make_shared<NodeEditor>();
  auto inspector2D = std::make_shared<Inspector2D>();

  children.push_back(std::dynamic_pointer_cast<ImGuiWidget>(nodeEditor));
  children.push_back(std::dynamic_pointer_cast<ImGuiWidget>(inspector2D));
}

void Application::Run() {
  bool shouldClose = false;
  while (!shouldClose) {
    glfwPollEvents();
    glfwMakeContextCurrent(glfwWindow);
    ImGuiIO& io = ImGui::GetIO();
    ImVec4 cc = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto child : children) {
      child->ImGuiDraw();
    }

    ImGui::ShowMetricsWindow();
    ImGui::ShowDemoWindow();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(glfwWindow, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(cc.x, cc.y, cc.z, cc.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

    glfwSwapBuffers(glfwWindow);
    shouldClose = glfwWindowShouldClose(glfwWindow);
  }
}

const nlohmann::json& Application::GetConfig() { return appConfig; }

GLuint Application::GetTexture(const std::string& name) {
  return textures[name];
}

void Application::LoadResources() {
  CreateFontAtlas();
  CreateTextures();
}

void Application::CreateFontAtlas() {
  ImGuiIO& io = ImGui::GetIO();

  IM_DELETE(io.Fonts);

  io.Fonts = IM_NEW(ImFontAtlas);

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

void LoadIcon(const char* filename, GLuint* textureId, ImVec2* size = nullptr) {
  // Load from file
  auto img = cv::imread(filename, cv::IMREAD_UNCHANGED);
  if (img.empty()) {
    throw std::runtime_error("Failed to load texture file");
  }

  if (img.channels() == 3) {
    cv::cvtColor(img, img, cv::COLOR_BGR2RGBA);
  } else if (img.channels() != 4) {
    throw std::runtime_error("must be rgb or rgba image");
  }

  // if (size > 0) {
  //   cv::resize(img, img, cv::Size(size, size));
  // }

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE);  // This is required on WebGL for non
                                      // power-of-two textures
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  //

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.cols, img.rows, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, img.data);
  glBindTexture(GL_TEXTURE_2D, 0);
  *textureId = image_texture;
  if (size != nullptr) *size = ImVec2(img.cols, img.rows);
}

void Application::CreateTextures() {
  auto iconsConfig = appConfig["resources"]["icons"];
  for (auto& icon : iconsConfig.items()) {
    auto iconId = icon.key();
    auto iconConfig = icon.value();
    auto iconFile = iconConfig["file"].get<std::string>();
    auto iconSize = iconConfig["size"].get<int>();

    GLuint textureId;
    LoadIcon(iconFile.c_str(), &textureId);
    textures[iconId] = textureId;
  }
}