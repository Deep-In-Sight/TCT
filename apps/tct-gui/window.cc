#include "window.h"

#include <backends/imgui_impl_opengl3.h>

#include "imgui-widget.h"
#include "imgui_impl_glfw.h"
#include "utility.h"

int Window::windowCount_ = 0;

Window::Window(const std::string& title, int width, int height, int x, int y,
               bool vsync)
    : title_(title) {
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

Window::~Window() {
  windowCount_--;

  if (windowCount_ == 0) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
  }

  ImGui::DestroyContext(imguiContext_);
  glfwDestroyWindow(glfwWindow_);
}

void Window::AddChild(std::shared_ptr<ImGuiWidget> child) {
  children_.push_back(child);
}

void Window::RemoveChild(std::shared_ptr<ImGuiWidget> child) {
  children_.erase(std::remove(children_.begin(), children_.end(), child),
                  children_.end());
}

bool Window::Render() {
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
