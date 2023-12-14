#pragma once
#include <GLFW/glfw3.h>
#include <SDL.h>

#include <map>
#include <memory>
#include <vector>

struct ImGuiContext;
struct ImGuiWidget;
struct Window;

enum BackEndType { BACKEND_GLFW = 1, BACKEND_SDL2, BACKEND_LAST };
enum WindowFlags { WindowFlags_VSYNC = 1, WindowFlags_DEFAULT = 0 };
struct BackEnd {
  static std::shared_ptr<Window> CreateWindow(const std::string& title, int x,
                                              int y, int width, int height,
                                              int flags = 0);
  static void Initialize(const BackEndType backend);
  static void Destroy();
  static void ProcessEvents();

 private:
  static BackEndType backend_;
  static bool initialized_;
  static void InitializeGlfw();
  static void TerminateGlfw();
  static void InitializeSdl2();
  static void TerminateSdl2();
};

struct Window {
  Window(const std::string& title);
  ~Window();

  void AddChild(std::shared_ptr<ImGuiWidget> child);
  void RemoveChild(std::shared_ptr<ImGuiWidget> child);

  virtual bool Render() = 0;

  std::vector<std::shared_ptr<ImGuiWidget>> children_;
  ImGuiContext* imguiContext_;
  std::string title_;
};

struct GlfwWindow : public Window {
  GlfwWindow(const std::string& title, int x, int y, int width, int height,
             int flags = 0);
  ~GlfwWindow();

  bool Render() override;

  GLFWwindow* glfwWindow_;
  static int windowCount_;
};

struct SdlWindow : public Window {
  SdlWindow(const std::string& title, int x, int y, int width, int height,
            int flags = 0);
  ~SdlWindow();

  bool Render() override;
  static void Close(uint32_t windowId);

  bool done_;
  SDL_Window* sdlWindow_;
  SDL_GLContext glContext_;
  int x_, y_;
  bool firstRender_;
  static int windowCount_;
  static std::map<uint32_t, SdlWindow*> windowList_;
};
