#include "window.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "imgui-widget.h"
#include "utility.h"

void WindowFocusCallback(GLFWwindow* window, int focused) {
  ImGuiContext* ctx = (ImGuiContext*)glfwGetWindowUserPointer(window);
  ImGui::SetCurrentContext(ctx);
  ImGui_ImplGlfw_WindowFocusCallback(window, focused);
}
void CursorEnterCallback(GLFWwindow* window, int entered) {
  ImGuiContext* ctx = (ImGuiContext*)glfwGetWindowUserPointer(window);
  ImGui::SetCurrentContext(ctx);
  ImGui_ImplGlfw_CursorEnterCallback(window, entered);
}
void CursorPosCallback(GLFWwindow* window, double x, double y) {
  ImGuiContext* ctx = (ImGuiContext*)glfwGetWindowUserPointer(window);
  ImGui::SetCurrentContext(ctx);
  ImGui_ImplGlfw_CursorPosCallback(window, x, y);
}
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  ImGuiContext* ctx = (ImGuiContext*)glfwGetWindowUserPointer(window);
  ImGui::SetCurrentContext(ctx);
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  ImGuiContext* ctx = (ImGuiContext*)glfwGetWindowUserPointer(window);
  ImGui::SetCurrentContext(ctx);
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}
void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods) {
  ImGuiContext* ctx = (ImGuiContext*)glfwGetWindowUserPointer(window);
  ImGui::SetCurrentContext(ctx);
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}
void CharCallback(GLFWwindow* window, unsigned int c) {
  ImGuiContext* ctx = (ImGuiContext*)glfwGetWindowUserPointer(window);
  ImGui::SetCurrentContext(ctx);
  ImGui_ImplGlfw_CharCallback(window, c);
}

void installUserCallbacks(GLFWwindow* window) {
  glfwSetWindowFocusCallback(window, WindowFocusCallback);
  glfwSetCursorEnterCallback(window, CursorEnterCallback);
  glfwSetCursorPosCallback(window, CursorPosCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSetScrollCallback(window, ScrollCallback);
  glfwSetKeyCallback(window, KeyCallback);
  glfwSetCharCallback(window, CharCallback);
}

Window::Window(const std::string& title, int width, int height, int x, int y,
               bool vsync) {
  // Create a window
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // no exception
  glfwWindow_ =
      glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!glfwWindow_) {
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwSwapInterval(vsync ? 1 : 0);

  glfwSetWindowPos(glfwWindow_, x, y);
  glfwMakeContextCurrent(glfwWindow_);

  int display_w, display_h;
  ImVec4 cc = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
  glfwGetFramebufferSize(glfwWindow_, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(cc.x, cc.y, cc.z, cc.w);

  // Initialize ImGui
  IMGUI_CHECKVERSION();

  imguiContext_ = ImGui::CreateContext();
  ImGui::SetCurrentContext(imguiContext_);

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(glfwWindow_, false);
  glfwSetWindowUserPointer(glfwWindow_, (void*)imguiContext_);
  installUserCallbacks(glfwWindow_);

  ImGui_ImplOpenGL3_Init("#version 130");

  CreateFontAtlas();
}

Window::~Window() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

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
  ImGui::SetCurrentContext(imguiContext_);
  glfwMakeContextCurrent(glfwWindow_);

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
