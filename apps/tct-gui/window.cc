#include "window.h"

#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <map>

#include "imgui-widget.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_sdl2.h"
#include "utility.h"

BackEndType BackEnd::backend_ = BACKEND_GLFW;
bool BackEnd::initialized_ = false;

void BackEnd::Initialize(BackEndType backend) {
  backend_ = backend;
  if (backend == BACKEND_GLFW) {
    InitializeGlfw();
  } else if (backend == BACKEND_SDL2) {
    InitializeSdl2();
  } else {
    throw std::runtime_error("Unknown backend");
  }
  initialized_ = true;
}

void BackEnd::Destroy() {
  if (backend_ == BACKEND_GLFW) {
    TerminateGlfw();
  } else if (backend_ == BACKEND_SDL2) {
    TerminateSdl2();
  }
  initialized_ = false;
}

void BackEnd::ProcessEvents() {
  if (!initialized_) {
    throw std::runtime_error("BackEnd not initialized");
  }

  if (backend_ == BACKEND_GLFW) {
    glfwPollEvents();
  } else if (backend_ == BACKEND_SDL2) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE) {
        SdlWindow::Close(event.window.windowID);
      }
    }
  }
}

std::shared_ptr<Window> BackEnd::CreateWindow(const std::string& title, int x,
                                              int y, int width, int height,
                                              int flags) {
  if (!initialized_) {
    throw std::runtime_error("BackEnd not initialized");
  }

  if (backend_ == BACKEND_GLFW) {
    return std::make_shared<GlfwWindow>(title, x, y, width, height, flags);
  } else if (backend_ == BACKEND_SDL2) {
    return std::make_shared<SdlWindow>(title, x, y, width, height, flags);
  } else {
    throw std::runtime_error("Unknown backend");
  }
}

void BackEnd::InitializeGlfw() {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
}

void BackEnd::TerminateGlfw() { glfwTerminate(); }

void BackEnd::InitializeSdl2() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
      0) {
    throw std::runtime_error("Failed to initialize SDL2");
  }
}

void BackEnd::TerminateSdl2() { SDL_Quit(); }

Window::Window(const std::string& title) : title_(title) {}
Window::~Window() {}

void Window::AddChild(std::shared_ptr<ImGuiWidget> child) {
  children_.push_back(child);
}

void Window::RemoveChild(std::shared_ptr<ImGuiWidget> child) {
  children_.erase(std::remove(children_.begin(), children_.end(), child),
                  children_.end());
}

int GlfwWindow::windowCount_ = 0;
GlfwWindow::GlfwWindow(const std::string& title, int x, int y, int width,
                       int height, int flags)
    : Window(title) {
  // Create a window
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

  // no exception
  glfwWindow_ =
      glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!glfwWindow_) {
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwSetWindowPos(glfwWindow_, x, y);
  glfwMakeContextCurrent(glfwWindow_);
  glfwShowWindow(glfwWindow_);

  bool vsync = (flags & WindowFlags_VSYNC);

  glfwSwapInterval(vsync ? 1 : 0);

  int display_w, display_h;
  ImVec4 cc = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
  glfwGetFramebufferSize(glfwWindow_, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(cc.x, cc.y, cc.z, cc.w);

  // Initialize ImGui
  IMGUI_CHECKVERSION();

  auto oldContext = ImGui::GetCurrentContext();
  imguiContext_ = ImGui::CreateContext();
  ImGui::SetCurrentContext(imguiContext_);

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  ImGui::StyleColorsDark();

  glfwSetWindowUserPointer(glfwWindow_, (void*)imguiContext_);
  ImGui_ImplGlfw_InitForOpenGL(glfwWindow_, true);

  ImGui_ImplOpenGL3_Init("#version 130");

  CreateFontAtlas();

  ImGui::SetCurrentContext(oldContext);

  windowCount_++;
}

GlfwWindow::~GlfwWindow() {
  windowCount_--;

  if (windowCount_ == 0) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
  }

  ImGui::DestroyContext(imguiContext_);
  glfwDestroyWindow(glfwWindow_);
}

bool GlfwWindow::Render() {
  glfwMakeContextCurrent(glfwWindow_);
  ImGui::SetCurrentContext(imguiContext_);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  for (auto& child : children_) {
    child->ImGuiDraw();
  }

  ImGui::Render();

  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();

  glfwSwapBuffers(glfwWindow_);

  return glfwWindowShouldClose(glfwWindow_);
}

int SdlWindow::windowCount_ = 0;
std::map<uint32_t, SdlWindow*> SdlWindow::windowList_;

SdlWindow::SdlWindow(const std::string& title, int x, int y, int width,
                     int height, int flags)
    : Window(title) {
  const char* glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                        SDL_WINDOW_ALLOW_HIGHDPI);
  sdlWindow_ =
      SDL_CreateWindow(title.c_str(), x, y, width, height, window_flags);
  if (sdlWindow_ == nullptr) {
    throw std::runtime_error(std::string("Error: SDL_CreateWindow(): ") +
                             SDL_GetError());
  }
  glContext_ = SDL_GL_CreateContext(sdlWindow_);
  SDL_GL_MakeCurrent(sdlWindow_, glContext_);
  int vsync = (flags & WindowFlags_VSYNC) ? 1 : 0;
  SDL_GL_SetSwapInterval(vsync);  // Enable vsync

  windowIcon_ = SDL_LoadBMP("./data/icons/appicon.bmp");
  if (windowIcon_ == nullptr) {
    throw std::runtime_error(std::string("Error: SDL_LoadBMP(): ") +
                             SDL_GetError());
  }
  SDL_SetWindowIcon(sdlWindow_, windowIcon_);

  auto oldContext = ImGui::GetCurrentContext();
  imguiContext_ = ImGui::CreateContext();
  ImGui::SetCurrentContext(imguiContext_);

  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport
                                                       // / Platform Windows
  ImGui::StyleColorsDark();

  int display_w, display_h;
  ImVec4 cc = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
  SDL_GetWindowSize(sdlWindow_, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(cc.x, cc.y, cc.z, cc.w);

  ImGui_ImplSDL2_InitForOpenGL(sdlWindow_, glContext_);
  ImGui_ImplOpenGL3_Init(glsl_version);

  CreateFontAtlas();
  ImGui::SetCurrentContext(oldContext);
  windowCount_++;
  done_ = false;
  windowList_[SDL_GetWindowID(sdlWindow_)] = this;
}

SdlWindow::~SdlWindow() {
  windowCount_--;
  if (windowCount_ == 0) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
  }
  ImGui::DestroyContext(imguiContext_);
  SDL_GL_DeleteContext(glContext_);
  SDL_DestroyWindow(sdlWindow_);
}

bool SdlWindow::Render() {
  SDL_GL_MakeCurrent(sdlWindow_, glContext_);
  ImGui::SetCurrentContext(imguiContext_);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  for (auto& child : children_) {
    child->ImGuiDraw();
  }

  ImGui::Render();

  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  SDL_GL_SwapWindow(sdlWindow_);

  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();

  return done_;
}

void SdlWindow::Close(uint32_t windowID) {
  windowList_[windowID]->done_ = true;
}