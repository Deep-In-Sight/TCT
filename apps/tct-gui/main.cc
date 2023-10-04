#include "application.h"

int main(int argc, char** argv) {
  Application& app = Application::GetInstance();

  app.Create();
  app.Run();

  return 0;
}