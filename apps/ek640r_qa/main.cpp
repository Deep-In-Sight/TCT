
#include <QtWidgets/QApplication>

#include "TopLevelNodeEditorWidget.hpp"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  auto mainWidget = new TopLevelNodeEditorWidget();
  mainWidget->showNormal();

  return app.exec();
}
