#pragma once

#include <QtNodes/NodeData>

using QtNodes::NodeData;
using QtNodes::NodeDataType;

/// The class can potentially incapsulate any user data which
/// need to be transferred within the Node Editor graph
class RawData : public NodeData {
 public:
  NodeDataType type() const override {
    return NodeDataType{"C4_16SC1", "C4_16SC1"};
  }
};

class DepthAmplitudeData : public NodeData {
 public:
  NodeDataType type() const override {
    return NodeDataType{"C2_32FC1", "C2_32FC1"};
  }
};

class DepthData : public NodeData {
 public:
  NodeDataType type() const override {
    return NodeDataType{"C1_32FC1", "C1_32FC1"};
  }
};

class AnyData : public NodeData {
 public:
  NodeDataType type() const override { return NodeDataType{"Any", "Any"}; }
};