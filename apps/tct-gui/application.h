#pragma once

#include <list>
#include <memory>
#include <string>

#include "nlohmann/json.hpp"

struct Window;
struct NodeEditor;

struct Application {
  static Application& GetInstance();
  ~Application();

  void Create();
  void Run();
  const nlohmann::json& GetConfig();
  void AddWindow(std::shared_ptr<Window> window);
  void RemoveWindow(std::shared_ptr<Window> window);
  std::shared_ptr<Window> GetWindow(const std::string& title);

  NodeEditor* nodeEditor;

 private:
  Application();

  nlohmann::json appConfig;

  std::list<std::shared_ptr<Window>> children;
};