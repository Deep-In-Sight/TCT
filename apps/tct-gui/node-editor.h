#pragma once
#include <imgui_node_editor.h>

#include <list>
#include <memory>
#include <vector>

#include "imgui-widget.h"
#include "utility.h"

struct ElementWrapper;
struct LinkInfo;

namespace ed = ax::NodeEditor;
struct NodeEditor : public ImGuiWidget {
  NodeEditor();
  ~NodeEditor();

  void ImGuiDraw() override;
  void ImGuiLayout() override;

  void AddNode(std::shared_ptr<ElementWrapper> node);

  void DrawNodes();
  void DrawLinks();
  void HandleLinkCreation();
  void HandleLinkDeletion();
  void HandleNodeDeletion();

  void ShowCreateNodePopup();
  bool doLink(ed::PinId startPinId, ed::PinId endPinId);
  bool doUnlink(ed::LinkId linkId);

  ed::EditorContext* m_Context;

  int windowFlags;

  std::list<std::shared_ptr<ElementWrapper>> m_Nodes_;
  std::list<LinkInfo> m_Links_;
};