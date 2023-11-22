#pragma once

#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

struct Window;

struct Application {
  static Application& GetInstance();
  ~Application();

  void Create();
  void Run();
  const nlohmann::json& GetConfig();

 private:
  Application();

  nlohmann::json appConfig;

  std::vector<std::shared_ptr<Window>> children;
};