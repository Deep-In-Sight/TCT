

#include "TopLevelNodeEditorWidget.hpp"

static std::shared_ptr<NodeDelegateModelRegistry> registerDataModels() {
  auto ret = std::make_shared<NodeDelegateModelRegistry>();
  ret->registerModel<PlaybackSourceNode>("Sources");
  ret->registerModel<DepthCalcNode>("ToF");
  ret->registerModel<VideoSinkNode>("Sinks");

  return ret;
}

TopLevelNodeEditorWidget::TopLevelNodeEditorWidget(QWidget *parent)
    : QWidget(parent) {
  std::shared_ptr<NodeDelegateModelRegistry> registry = registerDataModels();
  auto menuBar = new QMenuBar();
  QMenu *menu = menuBar->addMenu("File");
  auto saveAction = menu->addAction("Save Scene");
  auto loadAction = menu->addAction("Load Scene");
  QVBoxLayout *l = new QVBoxLayout(this);
  dataFlowGraphModel = new PipelineModel(registry);
  l->addWidget(menuBar);
  auto scene = new DataFlowGraphicsScene(*dataFlowGraphModel, this);
  auto view = new GraphicsView(scene);
  l->addWidget(view);
  l->setContentsMargins(0, 0, 0, 0);
  l->setSpacing(0);
  QObject::connect(saveAction, &QAction::triggered, scene,
                   &DataFlowGraphicsScene::save);

  QObject::connect(loadAction, &QAction::triggered, scene,
                   &DataFlowGraphicsScene::load);

  QObject::connect(scene, &DataFlowGraphicsScene::sceneLoaded, view,
                   &GraphicsView::centerScene);
  setWindowTitle("EK640R QA Tool");
  resize(1280, 720);

  labelLogo = new QLabel(this);
  // set a image for the label
  auto logoPixmap = QPixmap(":/images/logo_154x300.png");
  labelLogo->setPixmap(logoPixmap);
  labelLogo->resize(logoPixmap.size());

  int posX = width() - labelLogo->width() - 10;
  int posY = height() - labelLogo->height() - 10;
  labelLogo->setGeometry(posX, posY, labelLogo->width(), labelLogo->height());
}

void TopLevelNodeEditorWidget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);

  int posX = width() - labelLogo->width() - 10;  // Adjust the offset as needed
  int posY =
      height() - labelLogo->height() - 10;  // Adjust the offset as needed

  labelLogo->setGeometry(posX, posY, labelLogo->width(), labelLogo->height());
}