#include "PipelineModel.hpp"

#include <sdk/core/element.h>
#include <sdk/core/pad.h>

#include "NodeBase.hpp"

void PipelineModel::addConnection(QtNodes::ConnectionId const connectionId) {
  DataFlowGraphModel::addConnection(connectionId);
  auto sourceNode = _models.find(connectionId.outNodeId);
  auto sinkNode = _models.find(connectionId.inNodeId);
  if (sourceNode != _models.end() && sinkNode != _models.end()) {
    NodeBase* srcNodeBase = dynamic_cast<NodeBase*>(sourceNode->second.get());
    NodeBase* sinkNodeBase = dynamic_cast<NodeBase*>(sinkNode->second.get());

    Element* source = srcNodeBase->getElement();
    Element* sink = sinkNodeBase->getElement();
    source->GetPad("src")->Link(sink->GetPad("sink"));
  }
}

bool PipelineModel::connectionPossible(
    QtNodes::ConnectionId const connectionId) const {
  auto getDataType = [&](PortType const portType) {
    return portData(getNodeId(portType, connectionId), portType,
                    getPortIndex(portType, connectionId), PortRole::DataType)
        .value<NodeDataType>();
  };

  auto portVacant = [&](PortType const portType) {
    NodeId const nodeId = getNodeId(portType, connectionId);
    PortIndex const portIndex = getPortIndex(portType, connectionId);
    auto const connected = connections(nodeId, portType, portIndex);

    auto policy =
        portData(nodeId, portType, portIndex, PortRole::ConnectionPolicyRole)
            .value<ConnectionPolicy>();

    return connected.empty() || (policy == ConnectionPolicy::Many);
  };

  bool data_allow =
      (getDataType(PortType::Out).id == getDataType(PortType::In).id) ||
      (getDataType(PortType::Out).id == QString("Any")) ||
      (getDataType(PortType::In).id == QString("Any"));

  bool policy_allow = portVacant(PortType::Out) && portVacant(PortType::In);

  return data_allow && policy_allow;
}