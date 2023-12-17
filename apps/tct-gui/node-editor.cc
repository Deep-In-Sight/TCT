

#include "node-editor.h"

#include <ImGuiFileDialog.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <iostream>
#include <opencv2/imgcodecs/imgcodecs.hpp>

#include "IconsFontAwesome5.h"
#include "element-wrapper.h"
#include "inspector-bitmap-view.h"
#include "nlohmann/json.hpp"
#include "utility.h"

#ifdef _MSC_VER
#define portable_strcpy strcpy_s
#define portable_sprintf sprintf_s
#else
#define portable_strcpy strcpy
#define portable_sprintf sprintf
#endif

using NodeID = ed::NodeId;
using PinID = ed::PinId;
using LinkID = ed::LinkId;

NodeEditor::NodeEditor() {
  // ed::Config config;
  // config.SettingsFile = "Widgets.json";
  m_Context = ed::CreateEditor(nullptr);
  windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoBringToFrontOnFocus;

  // create a initial pipeline
  auto source = ElementFactory::CreateElement("PlayBack");
  auto depth = ElementFactory::CreateElement("RawToDepth");
  auto depthCalc = dynamic_cast<DepthCalc*>(depth->element_.get());
  depthCalc->SetConfig(37e6, 1.4);
  auto ma = ElementFactory::CreateElement("MovingAverage");
  // auto unprojection = ElementFactory::CreateElement("Unprojection");
  auto vis = ElementFactory::CreateElement("Open3DVisualizer");
  m_Nodes_.push_back(source);
  m_Nodes_.push_back(depth);
  m_Nodes_.push_back(ma);
  // m_Nodes_.push_back(unprojection);
  m_Nodes_.push_back(vis);

  source->SetLocation(ImVec2(100, 100));
  depth->SetLocation(ImVec2(400, 100));
  ma->SetLocation(ImVec2(700, 100));
  // unprojection->SetLocation(ImVec2(1000, 100));
  // vis->SetLocation(ImVec2(1300, 100));
  vis->SetLocation(ImVec2(1000, 100));

  auto sourceOutputPin = ed::PinId(&source->outputPads_.front());
  auto depthInputPin = ed::PinId(&depth->inputPads_.front());
  auto depthOutputPin = ed::PinId(&depth->outputPads_.front());
  auto maInputPin = ed::PinId(&ma->inputPads_.front());
  auto maOutputPin = ed::PinId(&ma->outputPads_.front());
  // auto unprojectionInputPin = ed::PinId(&unprojection->inputPads_.front());
  // auto unprojectionOutputPin = ed::PinId(&unprojection->outputPads_.front());
  auto visInputPin = ed::PinId(&vis->inputPads_.front());

  doLink(sourceOutputPin, depthInputPin);
  doLink(depthOutputPin, maInputPin);
  // doLink(maOutputPin, unprojectionInputPin);
  // doLink(unprojectionOutputPin, visInputPin);
  doLink(maOutputPin, visInputPin);
  m_Links_.emplace_back(sourceOutputPin, depthInputPin);
  m_Links_.emplace_back(depthOutputPin, maInputPin);
  // m_Links_.emplace_back(maOutputPin, unprojectionInputPin);
  // m_Links_.emplace_back(unprojectionOutputPin, visInputPin);
  m_Links_.emplace_back(maOutputPin, visInputPin);
}

NodeEditor::~NodeEditor() { ed::DestroyEditor(m_Context); }

void NodeEditor::ImGuiDraw() {
  static bool firstframe = true;  // Used to position the nodes on startup
  auto& io = ImGui::GetIO();

  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
  auto editorSize = ImGui::GetMainViewport()->WorkSize;
  editorSize.x *= 0.75f;
  ImGui::SetNextWindowSize(editorSize);
  ImGui::Begin("NodeEditor", nullptr, windowFlags);

  ImGui::Text("Node Editor");
  ImGui::Separator();

  // Node Editor Widget
  ed::SetCurrentEditor(m_Context);

  ed::Begin("My Editor", ImVec2(0.0, 0.0f));

  if (firstframe) {
    ed::NavigateToContent();
    firstframe = false;
  }

  DrawNodes();

  DrawLinks();

  HandleLinkCreation();

  HandleLinkDeletion();

  HandleNodeDeletion();

  ed::Suspend();
  if (ed::ShowBackgroundContextMenu()) {
    ImGui::OpenPopup("Create New Node");
    auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(io.MousePos);
  }
  ShowCreateNodePopup();
  ed::Resume();

  ed::End();
  ed::SetCurrentEditor(nullptr);
  firstframe = false;

  ImGui::End();  // end node editor window

  ImGui::ShowMetricsWindow();
  ImGui::ShowDemoWindow();
}

void NodeEditor::ImGuiLayout() {}

void NodeEditor::AddNode(std::shared_ptr<ElementWrapper> node) {
  m_Nodes_.push_back(node);
}

void NodeEditor::DrawNodes() {
  for (auto& node : m_Nodes_) {
    node->Draw();
  }
}

void NodeEditor::DrawLinks() {
  for (auto& linkInfo : m_Links_) {
    ed::LinkId id = ed::LinkId(&linkInfo);
    ed::Link(id, linkInfo.startPinId_, linkInfo.endPinId_);
  }
}

void NodeEditor::HandleLinkCreation() {
  if (!ed::BeginCreate()) {
    return;
  }

  ed::PinId startId, endId;
  if (!ed::QueryNewLink(&startId, &endId)) {
    ed::EndCreate();
    return;
  }

  if (!startId || !endId) {
    ed::EndCreate();
    return;
  }

  if (ed::AcceptNewItem()) {
    if (doLink(startId, endId)) {
      m_Links_.emplace_back(startId, endId);
    } else {
      ed::RejectNewItem();
    }
  }

  ed::EndCreate();
}

bool NodeEditor::doLink(ed::PinId startId, ed::PinId endId) {
  Pad* startPad = nullptr;
  Pad* endPad = nullptr;
  bool bothPads = false;
  PadObserver* observer = nullptr;

  startPad = ((PadWrapper*)startId.AsPointer())->pad_;
  endPad = ((PadWrapper*)endId.AsPointer())->pad_;

  // both nodes are observer
  if (!startPad && !endPad) {
    return false;
  }

  // both nodes are not observer
  if (startPad && endPad) {
    if (startPad->GetDirection() == endPad->GetDirection()) {
      return false;
    }
    if (startPad->GetDirection() == PadDirection::kPadSink) {
      std::swap(startId, endId);
      std::swap(startPad, endPad);
    }
    bothPads = true;
  } else {
    // one of two nodes is observer
    if (!startPad) {
      std::swap(startId, endId);
      std::swap(startPad, endPad);
    }
    auto endPadWrapper = (PadWrapper*)endId.AsPointer();
    observer = endPadWrapper->node_->observer_.get();
    bothPads = false;
  }

  if (bothPads) {
    startPad->Link(endPad);
  } else {
    startPad->AddObserver(observer);
  }

  return true;
}

void NodeEditor::HandleLinkDeletion() {
  if (!ed::BeginDelete()) {
    return;
  }

  ed::LinkId deletedLinkId;
  while (ed::QueryDeletedLink(&deletedLinkId)) {
    if (!ed::AcceptDeletedItem()) {
      ed::EndDelete();
      return;
    }

    for (auto linkInfo = m_Links_.begin(); linkInfo != m_Links_.end();
         linkInfo++) {
      ed::LinkId linkId = ed::LinkId(&(*linkInfo));
      if (linkId == deletedLinkId) {
        doUnlink(linkId);
        break;
      }
    }
  }
  ed::EndDelete();
}

bool NodeEditor::doUnlink(ed::LinkId linkId) {
  ed::PinId startPin, endPin;
  LinkInfo* linkInfo = (LinkInfo*)linkId.AsPointer();
  startPin = linkInfo->startPinId_;
  endPin = linkInfo->endPinId_;

  Pad* startPad = ((PadWrapper*)startPin.AsPointer())->pad_;
  Pad* endPad = ((PadWrapper*)endPin.AsPointer())->pad_;
  if (endPad) {
    startPad->Unlink();
  } else {
    auto endPadWrapper = (PadWrapper*)endPin.AsPointer();
    auto observer = endPadWrapper->node_->observer_.get();
    startPad->RemoveObserver(observer);
  }
  m_Links_.remove(*linkInfo);
  return true;
}

void NodeEditor::HandleNodeDeletion() {
  if (ed::BeginDelete()) {
    ed::NodeId deletedNodeId;
    while (ed::QueryDeletedNode(&deletedNodeId)) {
      if (ed::AcceptDeletedItem()) {
        for (auto iter = m_Nodes_.begin(); iter != m_Nodes_.end(); iter++) {
          ed::NodeId nodeId = ed::NodeId((*iter).get());
          if (nodeId == deletedNodeId) {
            m_Nodes_.erase(iter);
            break;
          }
        }
      }
    }
    ed::EndDelete();
  }
}

void NodeEditor::ShowCreateNodePopup() {
  auto& nodeTypes = GetNodeTypes();
  static ImGuiTextFilter filter;

  if (ImGui::BeginPopupModal("Create New Node")) {
    filter.Draw("Enter node type", 180.0f);
    // static all we want because there will be only one node editor

    for (int i = 0; i < nodeTypes.size(); i++) {
      if (filter.PassFilter(nodeTypes[i].c_str())) {
        if (ImGui::Selectable(nodeTypes[i].c_str())) {
          auto node = ElementFactory::CreateElement(nodeTypes[i]);
          AddNode(node);
          node->SetLocation(ed::ScreenToCanvas(ImGui::GetMousePos()));
        }
      }
    }

    if (ImGui::Button("Close")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}