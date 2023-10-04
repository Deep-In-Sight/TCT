#pragma once
#include <imgui_node_editor.h>

#include <list>
#include <memory>

#include "imgui-widget.h"
#include "node-editor.h"

namespace ed = ax::NodeEditor;

struct LinkInfo {
  ed::LinkId Id;
  ed::PinId InputId;
  ed::PinId OutputId;
};

struct NodeEditor : public ImGuiWidget {
  NodeEditor();
  ~NodeEditor();

  void ImGuiDraw() override;
  void ImGuiLayout() override;

  ed::EditorContext* m_Context;
  // List of live links. It is dynamic unless you
  // want to create read-only view over nodes.
  std::list<LinkInfo> m_Links;
  // Counter to help generate link ids. In real application this will
  // probably based on pointer to user data structure.
  int m_NextLinkId = 100;
  int windowFlags;
};