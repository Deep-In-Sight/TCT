

#include "node-editor.h"

#include <ImGuiFileDialog.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <iostream>
#include <opencv2/imgcodecs/imgcodecs.hpp>

#include "IconsFontAwesome5.h"
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
    ed::LinkId id = ed::LinkId(linkInfo.get());
    ed::Link(id, linkInfo->startPinId_, linkInfo->endPinId_);
  }
}

void NodeEditor::HandleLinkCreation() {
  Pad* startPad = nullptr;
  Pad* endPad = nullptr;
  bool bothPads = false;
  InspectorBitmapView* inspector = nullptr;

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

  startPad = ((PadWrapper*)startId.AsPointer())->pad_;
  endPad = ((PadWrapper*)endId.AsPointer())->pad_;

  // both nodes are observer
  if (!startPad && !endPad) {
    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
    ed::EndCreate();
    return;
  }

  // both nodes are not observer
  if (startPad && endPad) {
    if (startPad->GetDirection() == endPad->GetDirection()) {
      ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
      ed::EndCreate();
      return;
    }
    if (startPad->GetDirection() == PadDirection::kPadSink) {
      std::swap(startId, endId);
      std::swap(startPad, endPad);
      std::cout << "reversed" << std::endl;
    }
    bothPads = true;
  } else {
    // one of two nodes is observer
    if (!startPad) {
      std::swap(startId, endId);
      std::swap(startPad, endPad);
      std::cout << "reversed" << std::endl;
    }
    auto endPadWrapper = (PadWrapper*)endId.AsPointer();
    auto node = dynamic_cast<VideoOutputNode*>(endPadWrapper->node_);

    // only the video output node can be used as observer
    if (node) {
      inspector = node->inspector_.get();
    }
    bothPads = false;
  }

  if (ed::AcceptNewItem()) {
    auto link = std::make_shared<LinkInfo>(startId, endId);
    m_Links_.push_back(link);
    if (bothPads) {
      startPad->Link(endPad);
    } else {
      startPad->AddObserver(inspector);
    }
  }

  ed::EndCreate();
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

    for (auto iter = m_Links_.begin(); iter != m_Links_.end(); iter++) {
      ed::LinkId linkId = ed::LinkId((*iter).get());
      if (linkId == deletedLinkId) {
        ed::PinId startPin, endPin;
        startPin = (*iter)->startPinId_;
        endPin = (*iter)->endPinId_;
        Pad* startPad = ((PadWrapper*)startPin.AsPointer())->pad_;
        Pad* endPad = ((PadWrapper*)endPin.AsPointer())->pad_;
        if (endPad) {
          startPad->Unlink();
        } else {
          auto endPadWrapper = (PadWrapper*)endPin.AsPointer();
          auto node = dynamic_cast<VideoOutputNode*>(endPadWrapper->node_);
          startPad->RemoveObserver(node->inspector_.get());
        }
        m_Links_.erase(iter);
        break;
      }
    }
  }
  ed::EndDelete();
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
  const char* nodeTypes[] = {"Playback", "RawToDepth", "MovingAverage",
                             "VideoOutput"};
  static ImGuiTextFilter filter;

  if (ImGui::BeginPopupModal("Create New Node")) {
    filter.Draw("Enter node type", 180.0f);
    // static all we want because there will be only one node editor

    for (int i = 0; i < IM_ARRAYSIZE(nodeTypes); i++) {
      if (filter.PassFilter(nodeTypes[i])) {
        if (ImGui::Selectable(nodeTypes[i])) {
          auto node = ElementFactory::CreateElement(nodeTypes[i]);
          AddNode(node);
          node->SetLocation(ImGui::GetMousePos());
        }
      }
    }

    if (ImGui::Button("Close")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}