#pragma once
#include <imgui.h>
#include <imgui_node_editor.h>
#include <sdk/core/element.h>
#include <sdk/tof/depth-calc.h>
#include <sdk/tof/unprojection.h>

#include <list>
#include <memory>
#include <string>

struct PadWrapper;
struct InspectorBitmapView;
struct Open3DVisualizer;
struct NodeEditor;

namespace ed = ax::NodeEditor;

struct ElementWrapper {
  ElementWrapper();

  // parse the element to populate the inputPads and outputPads
  void BuildNode();
  // save Element settings to json string
  virtual void SaveState();
  // configure Element with json string
  virtual void LoadState(std::string& savedData);

  void Draw();
  virtual void DrawHeader();
  virtual void DrawInputPads();
  virtual void DrawBody();
  virtual void DrawOutputPads();
  void SetLocation(ImVec2 location);

  std::list<PadWrapper> inputPads_;
  std::list<PadWrapper> outputPads_;

  std::string nodeSettings_;

  bool firstFrame;
  ImVec2 location_;
  std::shared_ptr<Element> element_;
  std::shared_ptr<PadObserver> observer_;
  std::function<void()> deferredDraw_;
};

struct PadWrapper {
  PadWrapper(Pad* pad);
  Pad* pad_;
  ElementWrapper* node_;
};

struct LinkInfo {
  LinkInfo(ed::PinId startPinId, ed::PinId endPinId);
  ed::PinId startPinId_;
  ed::PinId endPinId_;
  bool operator==(const LinkInfo& other) const;
};

struct PlayBackNode : public ElementWrapper {
  PlayBackNode(const std::string& name, ImColor color = ImColor(255, 255, 255));
  void DrawBody() override;
  void SaveState() override;
  void LoadState(std::string& savedData) override;
  NodeEditor* nodeEditor_;
  bool showPresetPopup_;
  bool showTypePopup_;
  char fn_[256];
  bool usePreset_;
  int presetIdx_;
  MatShape shape_;
  int typeId_;
  float fps_;
  bool loop_;
  bool changed;
};

struct RawToDepthNode : public ElementWrapper {
  RawToDepthNode(const std::string& name,
                 ImColor color = ImColor(255, 255, 255));
  void DrawBody() override;
  void SaveState() override;
  void LoadState(std::string& savedData) override;
  float fmodMHz_;
  float offset_;
};

struct MovingAverageNode : public ElementWrapper {
  MovingAverageNode(const std::string& name,
                    ImColor color = ImColor(255, 255, 255));
  void DrawBody() override;
  void SaveState() override;
  void LoadState(std::string& savedData) override;
  int width;
};

struct UnprojectionNode : public ElementWrapper {
  UnprojectionNode(const std::string& name,
                   ImColor color = ImColor(255, 255, 255));
  void DrawBody() override;
  void SaveState() override;
  void LoadState(std::string& savedData) override;
  PinholeParams params_;
  bool usePreset_;
  int presetIdx_;
  bool showPresetPopup_;
  std::vector<std::string> presetNames_;
};

struct VideoOutputNode : public ElementWrapper {
  VideoOutputNode(const std::string& name,
                  ImColor color = ImColor(255, 255, 255));
  ~VideoOutputNode();
  void DrawHeader() override;
  void DrawInputPads() override;
};

struct Open3DVisualizerNode : public ElementWrapper {
  Open3DVisualizerNode(const std::string& name,
                       ImColor color = ImColor(255, 255, 255));
  void DrawHeader() override;
  void DrawInputPads() override;
  void DrawBody() override;
  bool showIntrinsics;
  bool showExtrinsics;
};

struct ElementFactory {
  static std::shared_ptr<ElementWrapper> CreateElement(
      const std::string& nodeType, const std::string& nodeName = "");
};

std::map<std::string,
         std::function<std::shared_ptr<ElementWrapper>(const std::string&)>>&
GetNodeConstructors();
const std::vector<std::string>& GetNodeTypes();
std::map<std::string, int>& GetNodesCount();