#ifndef __PIPELINE_MODEL_H__
#define __PIPELINE_MODEL_H__

#include <QtNodes/DataFlowGraphModel>

using QtNodes::ConnectionPolicy;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::NodeId;
using QtNodes::PortIndex;
using QtNodes::PortRole;
using QtNodes::PortType;

class PipelineModel : public QtNodes::DataFlowGraphModel {
  Q_OBJECT
 public:
  PipelineModel(std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry)
      : QtNodes::DataFlowGraphModel(registry) {}

  void addConnection(QtNodes::ConnectionId const connectionId) override;
  bool deleteConnection(QtNodes::ConnectionId const connectionId) override;
  bool connectionPossible(
      QtNodes::ConnectionId const connectionId) const override;
};

#endif  // __PIPELINE_MODEL_H__