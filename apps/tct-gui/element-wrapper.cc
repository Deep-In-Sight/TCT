#include "element-wrapper.h"

#include <ImGuiFileDialog.h>
#include <sdk/tof/depth-calc.h>
#include <sdk/tof/moving-average.h>
#include <sdk/tof/playback-src.h>
#include <sdk/tof/unprojection.h>

#include <nlohmann/json.hpp>

#include "IconsFontAwesome5.h"
#include "application.h"
#include "inspector-bitmap-view.h"
#include "open3d-visualizer.h"
#include "window.h"

LinkInfo::LinkInfo(ed::PinId startPinId, ed::PinId endPinId)
    : startPinId_(startPinId), endPinId_(endPinId) {}
bool LinkInfo::operator==(const LinkInfo& other) const {
  return startPinId_ == other.startPinId_ && endPinId_ == other.endPinId_;
}

PadWrapper::PadWrapper(Pad* pad) : pad_(pad) {}

ElementWrapper::ElementWrapper() : firstFrame(true), location_(20, 20) {}

void ElementWrapper::BuildNode() {
  for (Pad* p : element_->GetPads()) {
    PadWrapper padWrapper(p);
    padWrapper.node_ = this;
    if (p->GetDirection() == PadDirection::kPadSink) {
      inputPads_.push_back(padWrapper);
    } else {
      outputPads_.push_back(padWrapper);
    }
  }
}

void ElementWrapper::SaveState(){};
void ElementWrapper::LoadState(std::string& savedData){};

void ElementWrapper::Draw() {
  ed::NodeId id = ed::NodeId(this);
  ed::BeginNode(id);
  DrawHeader();
  DrawInputPads();
  ImGui::SameLine();
  DrawBody();
  ImGui::SameLine();
  DrawOutputPads();
  ed::EndNode();

  // the magical node editor does not allow combo box inside a node,
  // so we are forced to change it to a button and popup drawn after EndNode()
  ed::Suspend();
  if (deferredDraw_) {
    deferredDraw_();
  }
  ed::Resume();

  if (firstFrame) {
    ed::SetNodePosition(id, location_);
    firstFrame = false;
  }
}

void ElementWrapper::DrawHeader() {
  ImGui::TextUnformatted(element_->GetName().c_str());
  // other decorations
}

void ElementWrapper::DrawInputPads() {
  ImGui::BeginGroup();

  for (auto& padWrapper : inputPads_) {
    ed::PinId pinId = ed::PinId(&padWrapper);
    ed::BeginPin(pinId, ed::PinKind::Input);
    ImGui::TextUnformatted(ICON_FA_ARROW_RIGHT);
    ed::EndPin();
    // ImGui::SameLine();
    ImGui::TextUnformatted(padWrapper.pad_->GetName().c_str());
  }

  ImGui::EndGroup();
}

void ElementWrapper::DrawBody() {}

void ElementWrapper::DrawOutputPads() {
  ImGui::BeginGroup();

  for (auto& padWrapper : outputPads_) {
    ed::PinId pinId = ed::PinId(&padWrapper);
    // ImGui::SameLine();
    ed::BeginPin(pinId, ed::PinKind::Output);
    ImGui::TextUnformatted(ICON_FA_ARROW_RIGHT);
    ed::EndPin();
    ImGui::TextUnformatted(padWrapper.pad_->GetName().c_str());
  }

  ImGui::EndGroup();
}

void ElementWrapper::SetLocation(ImVec2 location) {
  location_ = location;
  firstFrame = true;
}

// there must be a way to do this smarter...

PlayBackNode::PlayBackNode(const std::string& name, ImColor color)
    : ElementWrapper() {
  auto playback = std::make_shared<PlaybackSource>(name, false, true);
  element_ = playback;
  BuildNode();
  fn_[0] = '\0';
  showPresetPopup_ = false;
  showTypePopup_ = false;
  usePreset_ = true;
  presetIdx_ = 0;
  shape_ = MatShape(4, 480, 640);  // =.= hard initialize
  typeId_ = CV_16SC1;
  fps_ = 30;
  loop_ = true;
  changed = false;
};

void presetIdToFormat(int presetIdx, MatShape& shape, int& type) {
  struct Format {
    MatShape shape;
    int type;
  };
  std::map<int, Format> presets = {{0, {{4, 480, 640}, CV_16SC1}},
                                   {1, {{4, 240, 320}, CV_16SC1}}};
  if (presets.count(presetIdx) > 0) {
    shape = presets[presetIdx].shape;
    type = presets[presetIdx].type;
  }
}

int idToType(int id) {
  std::map<int, int> idToType = {{0, CV_16SC1}, {1, CV_16UC1}};
  if (idToType.count(id) > 0) {
    return idToType[id];
  } else {
    return CV_16SC1;
  }
}

void PlayBackNode::DrawBody() {
  ImGui::BeginGroup();
  ImGui::PushID(element_->GetName().c_str());

  auto playback = dynamic_cast<PlaybackSource*>(element_.get());
  if (ImGui::Button(ICON_FA_FILE "Open")) {
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File",
                                            ".bin", ".");
    deferredDraw_ = [this]() {
      auto dialog = ImGuiFileDialog::Instance();
      auto playback = dynamic_cast<PlaybackSource*>(element_.get());
      if (dialog->Display("ChooseFileDlgKey")) {
        if (dialog->IsOk()) {
          std::string filePathName = dialog->GetFilePathName();
          std::string fileName = dialog->GetCurrentFileName();
          strcpy(fn_,
                 &fileName[fileName.size() - 20]);  // copy last 20 chars only
          playback->SetFilename(filePathName);
          deferredDraw_ = nullptr;
        }
        ImGuiFileDialog::Instance()->Close();
      }
    };
  };
  ImGui::SameLine();
  ImGui::PushItemWidth(150);
  ImGui::TextWrapped("...%s", fn_);
  ImGui::PopItemWidth();
  ImGui::PushItemWidth(50);
  ImGui::Checkbox("Use Preset", &usePreset_);
  if (usePreset_) {
    static const char* presets[] = {"4x480x640/CV_16SC1", "4x240x320/CV_16SC1"};
    showPresetPopup_ = false;
    ImGui::BeginGroup();
    if (ImGui::Button(presets[presetIdx_])) {
      showPresetPopup_ = true;
      deferredDraw_ = [this]() {
        if (showPresetPopup_) ImGui::OpenPopup("PresetPopup");
        if (ImGui::BeginPopup("PresetPopup")) {
          for (int i = 0; i < IM_ARRAYSIZE(presets); i++) {
            if (ImGui::Selectable(presets[i])) {
              presetIdx_ = i;
              presetIdToFormat(presetIdx_, shape_, typeId_);
              // playback->SetFormat(shape_, typeId_);
              changed = true;
            }
          }
          ImGui::EndPopup();
        }
      };
    }
  } else {
    int channels, height, width;
    channels = shape_[0];
    height = shape_[1];
    width = shape_[2];

    ImGui::BeginGroup();
    if (ImGui::DragInt("#Phase", &channels, 1, 1, 4) ||
        ImGui::DragInt("Height", &height, 1, 1, 480) ||
        ImGui::DragInt("Width", &width, 1, 1, 640)) {
      shape_ = MatShape(channels, height, width);
      // playback->SetFormat(MatShape(channels, height, width), typeId_);
      changed = true;
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();

    showTypePopup_ = false;
    if (ImGui::Button(cv::typeToString(typeId_).c_str())) {
      showTypePopup_ = true;
      deferredDraw_ = [&]() {
        if (showTypePopup_) ImGui::OpenPopup("TypePopup");
        if (ImGui::BeginPopup("TypePopup")) {
          static const char* types[] = {"CV_16SC1", "CV_16UC1"};
          for (int i = 0; i < IM_ARRAYSIZE(types); i++) {
            if (ImGui::Selectable(types[i])) {
              typeId_ = idToType(i);
              // playback->SetFormat(shape_, typeId_);
              changed = true;
            }
          }
          ImGui::EndPopup();
        }
      };
    }
  }

  if (changed) {
    playback->SetFormat(shape_, typeId_);
    changed = false;
  }

  if (ImGui::DragFloat("FPS", &fps_, 1, 1, 60, "%.2f")) {
    playback->SetFrameRate(fps_);
  }
  if (ImGui::Checkbox("Loop", &loop_)) {
    playback->SetLoop(loop_);
  }

  ImGui::PopItemWidth();
  ImGui::EndGroup();

  StreamState state = playback->GetState();
  // TODO: probably some issues if user click too fast
  if (state == StreamState::kStreamStateStopped) {
    if (ImGui::Button(ICON_FA_PLAY "Play")) {
      playback->Start();
    }
  } else if (state == StreamState::kStreamStatePlaying) {
    if (ImGui::Button(ICON_FA_PAUSE "Pause")) {
      playback->Pause();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_STOP "Stop")) {
      playback->Stop();
    }
  } else if (state == StreamState::kStreamStatePaused) {
    if (ImGui::Button(ICON_FA_PLAY "Resume")) {
      playback->Resume();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_STOP "Stop")) {
      playback->Stop();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_STEP_FORWARD "Step")) {
      playback->Step();
    }
  }
  ImGui::PopID();

  ImGui::EndGroup();
};

void PlayBackNode::SaveState() {
  nlohmann::json nodeSettings;

  nodeSettings["fn"] = fn_;
  nodeSettings["usePreset"] = usePreset_;
  nodeSettings["presetIdx"] = presetIdx_;
  nodeSettings["phases"] = shape_[0];
  nodeSettings["height"] = shape_[1];
  nodeSettings["width"] = shape_[2];
  nodeSettings["type"] = typeId_;
  nodeSettings["fps"] = fps_;
  nodeSettings["loop"] = loop_;

  nodeSettings_ = nodeSettings.dump();
};

void PlayBackNode::LoadState(std::string& savedData) {
  auto playback = dynamic_cast<PlaybackSource*>(element_.get());
  nlohmann::json nodeSettings = nlohmann::json::parse(savedData);

  auto filename = nodeSettings["fn"].get<std::string>();
  strcpy(fn_, filename.c_str());
  usePreset_ = nodeSettings["usePreset"].get<bool>();
  presetIdx_ = nodeSettings["presetIdx"].get<int>();
  shape_ = MatShape(nodeSettings["phases"].get<int>(),
                    nodeSettings["height"].get<int>(),
                    nodeSettings["width"].get<int>());
  typeId_ = nodeSettings["type"].get<int>();
  fps_ = nodeSettings["fps"].get<float>();
  loop_ = nodeSettings["loop"].get<bool>();

  playback->SetFilename(fn_);
  playback->SetFormat(shape_, typeId_);
  playback->SetFrameRate(fps_);
  playback->SetLoop(loop_);
};

RawToDepthNode::RawToDepthNode(const std::string& name, ImColor color)
    : ElementWrapper() {
  auto depthCalc = std::make_shared<DepthCalc>(name);
  element_ = depthCalc;
  BuildNode();
};

void RawToDepthNode::DrawBody() {
  auto depthCalc = dynamic_cast<DepthCalc*>(element_.get());
  ImGui::BeginGroup();
  ImGui::PushItemWidth(100);
  ImGui::PushID(element_->GetName().c_str());
  depthCalc->GetConfig(fmodMHz_, offset_);
  fmodMHz_ /= 1e6;
  if (ImGui::DragFloat("FmodMHz", &fmodMHz_, 0.1, 1, 100) ||
      ImGui::DragFloat("Offset", &offset_, 0.01, -10, 10)) {
    depthCalc->SetConfig(fmodMHz_ * 1e6, offset_);
  }
  ImGui::PopID();
  ImGui::PopItemWidth();
  ImGui::EndGroup();
};

void RawToDepthNode::SaveState() {
  nlohmann::json nodeSettings;

  nodeSettings["fmodMHz"] = fmodMHz_;
  nodeSettings["offset"] = offset_;

  nodeSettings_ = nodeSettings.dump();
};

void RawToDepthNode::LoadState(std::string& savedData) {
  auto depthCalc = dynamic_cast<DepthCalc*>(element_.get());
  nlohmann::json nodeSettings = nlohmann::json::parse(savedData);

  fmodMHz_ = nodeSettings["fmodMHz"].get<int>();
  offset_ = nodeSettings["offset"].get<float>();
  depthCalc->SetConfig(fmodMHz_ * 1e6, offset_);
};

MovingAverageNode::MovingAverageNode(const std::string& name, ImColor color)
    : ElementWrapper() {
  auto ma = std::make_shared<MovingAverage>(name);
  element_ = ma;
  BuildNode();
  width = 32;
  ma->SetWindowSize(width);
};

void MovingAverageNode::DrawBody() {
  auto movingAverage = dynamic_cast<MovingAverage*>(element_.get());
  ImGui::BeginGroup();
  ImGui::PushItemWidth(50);
  ImGui::PushID(element_->GetName().c_str());
  if (ImGui::DragInt("Window Size", &width, 1, 1, 64)) {
    movingAverage->SetWindowSize(width);
  }
  ImGui::PopID();
  ImGui::PopItemWidth();
  ImGui::EndGroup();
};

void MovingAverageNode::SaveState() {
  nlohmann::json nodeSettings;

  nodeSettings["width"] = width;

  nodeSettings_ = nodeSettings.dump();
};

void MovingAverageNode::LoadState(std::string& savedData) {
  nlohmann::json nodeSettings = nlohmann::json::parse(savedData);

  auto movingAverage = dynamic_cast<MovingAverage*>(element_.get());
  width = nodeSettings["width"].get<int>();
  movingAverage->SetWindowSize(width);
};

UnprojectionNode::UnprojectionNode(const std::string& name, ImColor color)
    : ElementWrapper() {
  auto unprojection = std::make_shared<Unprojection>(name);
  element_ = unprojection;
  params_ = unprojection->GetParams();
  usePreset_ = true;
  presetNames_ = PinholeParams::GetPresetNames();
  presetIdx_ = 0;
  BuildNode();
};

void UnprojectionNode::DrawBody() {
  auto unprojection = dynamic_cast<Unprojection*>(element_.get());
  ImGui::BeginGroup();
  ImGui::PushItemWidth(100);
  ImGui::PushID(element_->GetName().c_str());
  ImGui::TextUnformatted("Pinhole Parameters");
  ImGui::Checkbox("Use Preset", &usePreset_);
  if (usePreset_) {
    showPresetPopup_ = false;
    if (ImGui::Button(presetNames_[presetIdx_].c_str())) {
      showPresetPopup_ = true;
      deferredDraw_ = [this]() {
        if (showPresetPopup_) ImGui::OpenPopup("PinholePresetPopup");
        if (ImGui::BeginPopup("PinholePresetPopup")) {
          for (int i = 0; i < presetNames_.size(); i++) {
            if (ImGui::Selectable(presetNames_[i].c_str())) {
              presetIdx_ = i;
              auto unprojection = dynamic_cast<Unprojection*>(element_.get());
              params_ = PinholeParams::GetPreset(presetNames_[presetIdx_]);
              unprojection->SetParams(params_);
            }
          }
          ImGui::EndPopup();
        }
      };
    }
  } else {
    if (ImGui::DragFloat("focal x (mm)", &params_.fx_, 0.01, 0.0, 20.0,
                         "%.2f") ||
        ImGui::DragFloat("focal y (mm)", &params_.fy_, 0.01, 0.0, 20.0,
                         "%.2f") ||
        ImGui::DragFloat("Principal x (pixel)", &params_.cx_, 1, 0.0, 1000,
                         "%.0f") ||
        ImGui::DragFloat("Principal y (pixel)", &params_.cy_, 1, 0.0, 1000,
                         "%.0f") ||
        ImGui::DragFloat("Pixel width (um)", &params_.dx_, 0.01, 0.0, 100.0,
                         "%.2f") ||
        ImGui::DragFloat("Pixel height (um)", &params_.dy_, 0.01, 0.0, 100.0,
                         "%.2f")) {
      unprojection->SetParams(params_);
    }
  }

  ImGui::PopID();
  ImGui::PopItemWidth();
  ImGui::EndGroup();
}

void UnprojectionNode::LoadState(std::string& savedData) {
  auto unprojection = dynamic_cast<Unprojection*>(element_.get());
  nlohmann::json nodeSettings = nlohmann::json::parse(savedData);

  usePreset_ = nodeSettings["usePreset"].get<bool>();
  presetIdx_ = nodeSettings["presetIdx"].get<int>();
  float fx = nodeSettings["fx"].get<float>();
  float fy = nodeSettings["fy"].get<float>();
  float cx = nodeSettings["cx"].get<float>();
  float cy = nodeSettings["cy"].get<float>();
  float dx = nodeSettings["dx"].get<float>();
  float dy = nodeSettings["dy"].get<float>();
  if (usePreset_) {
    params_ = PinholeParams::GetPreset(presetNames_[presetIdx_]);
  } else {
    params_ = PinholeParams(fx, fy, cx, cy, dx, dy);
  }
  unprojection->SetParams(params_);
}

void UnprojectionNode::SaveState() {
  nlohmann::json nodeSettings;

  nodeSettings["usePreset"] = usePreset_;
  nodeSettings["presetIdx"] = presetIdx_;
  nodeSettings["fx"] = params_.fx_;
  nodeSettings["fy"] = params_.fy_;
  nodeSettings["cx"] = params_.cx_;
  nodeSettings["cy"] = params_.cy_;
  nodeSettings["dx"] = params_.dx_;
  nodeSettings["dy"] = params_.dy_;

  nodeSettings_ = nodeSettings.dump();
}

VideoOutputNode::VideoOutputNode(const std::string& name, ImColor color)
    : ElementWrapper() {
  inputPads_.emplace_back(nullptr);
  inputPads_.back().node_ = this;
  auto observerView_ = std::make_shared<InspectorBitmapView>(name);
  observer_ = observerView_;
  auto w = BackEnd::CreateWindow(observer_->GetName(), 0, 0, 1200, 720);
  auto& app = Application::GetInstance();
  app.AddWindow(w);
  w->AddChild(observerView_);
};

void VideoOutputNode::DrawHeader() {
  auto nodeName = observer_->GetName();
  ImGui::TextUnformatted(nodeName.c_str());
}

void VideoOutputNode::DrawInputPads() {
  ImGui::BeginGroup();
  ed::PinId pinId = ed::PinId(&inputPads_.front());
  ed::BeginPin(pinId, ed::PinKind::Input);
  ImGui::TextUnformatted(ICON_FA_EYE);
  ed::EndPin();
  ImGui::TextUnformatted("sink");
  ImGui::EndGroup();
}

VideoOutputNode::~VideoOutputNode() {
  auto& app = Application::GetInstance();
  auto w = app.GetWindow(observer_->GetName());
  app.RemoveWindow(w);
}

Open3DVisualizerNode::Open3DVisualizerNode(const std::string& name,
                                           ImColor color)
    : ElementWrapper() {
  auto observerView_ = std::make_shared<Open3DVisualizer>(name);
  observer_ = observerView_;
  inputPads_.emplace_back(nullptr);
  inputPads_.back().node_ = this;
  showIntrinsics = false;
};

void Open3DVisualizerNode::DrawHeader() {
  auto nodeName = observer_->GetName();
  ImGui::TextUnformatted(nodeName.c_str());
}

void Open3DVisualizerNode::DrawInputPads() {
  ImGui::BeginGroup();
  ed::PinId pinId = ed::PinId(&inputPads_.front());
  ed::BeginPin(pinId, ed::PinKind::Input);
  ImGui::TextUnformatted(ICON_FA_EYE);
  ed::EndPin();
  ImGui::TextUnformatted("sink");
  ImGui::EndGroup();
}

void Open3DVisualizerNode::DrawBody() {
  auto observer = std::dynamic_pointer_cast<Open3DVisualizer>(observer_);
  auto name = observer_->GetName();
  CameraIntrinsics intrinsics;
  observer->GetIntrinsics(intrinsics);
  ImGui::BeginGroup();
  ImGui::PushID(name.c_str());

  ImGui::Checkbox("Show Intrinsics", &showIntrinsics);
  if (showIntrinsics) {
    ImGui::PushItemWidth(100);
    if (ImGui::DragFloat("focal x (mm)", &intrinsics.fx, 0.01, 0.0, 20.0,
                         "%.2f") ||
        ImGui::DragFloat("focal y (mm)", &intrinsics.fy, 0.01, 0.0, 20.0,
                         "%.2f") ||
        ImGui::DragFloat("Principal x (pixel)", &intrinsics.cx, 1, -1000, 1000,
                         "%.0f") ||
        ImGui::DragFloat("Principal y (pixel)", &intrinsics.cy, 1, -1000, 1000,
                         "%.0f") ||
        ImGui::DragFloat("Pixel width (um)", &intrinsics.dx, 0.01, 0.0, 100.0,
                         "%.2f") ||
        ImGui::DragFloat("Pixel height (um)", &intrinsics.dy, 0.01, 0.0, 100.0,
                         "%.2f")) {
      observer->SetIntrinsics(intrinsics);
    }
    ImGui::PopItemWidth();
  }

  ImGui::PopID();
  ImGui::EndGroup();
}

using NodeConstructor =
    std::function<std::shared_ptr<ElementWrapper>(const std::string& name)>;

std::map<std::string, NodeConstructor>& GetNodeConstructors() {
  static std::map<std::string, NodeConstructor> constructorMap = {
      {"PlayBack",
       [](const std::string& name) {
         return std::make_shared<PlayBackNode>(name);
       }},
      {"RawToDepth",
       [](const std::string& name) {
         return std::make_shared<RawToDepthNode>(name);
       }},
      {"MovingAverage",
       [](const std::string& name) {
         return std::make_shared<MovingAverageNode>(name);
       }},
      {"Unprojection",
       [](const std::string& name) {
         return std::make_shared<UnprojectionNode>(name);
       }},
      {"VideoOutput",
       [](const std::string& name) {
         return std::make_shared<VideoOutputNode>(name);
       }},
      {"Open3DVisualizer", [](const std::string& name) {
         return std::make_shared<Open3DVisualizerNode>(name);
       }}};
  return constructorMap;
}

const std::vector<string>& GetNodeTypes() {
  static std::vector<string> nodeTypes;
  auto& constructorMap = GetNodeConstructors();
  if (nodeTypes.empty()) {
    for (const auto& nodeConstructor : constructorMap) {
      nodeTypes.push_back(nodeConstructor.first);
    }
    std::sort(nodeTypes.begin(), nodeTypes.end());
  }
  return nodeTypes;
}

std::map<std::string, int>& GetNodesCount() {
  static std::map<std::string, int> nodeCounts;
  const auto& types = GetNodeTypes();
  if (nodeCounts.empty()) {
    for (auto type : types) {
      nodeCounts[type] = 0;
    }
  }
  return nodeCounts;
}

std::shared_ptr<ElementWrapper> ElementFactory::CreateElement(
    const std::string& nodeType, const std::string& nodeName) {
  auto counts = GetNodesCount();
  auto constructorMap = GetNodeConstructors();

  std::string assignedName = nodeName;
  if (nodeName.empty() && counts.count(nodeType) > 0) {
    assignedName = nodeType + std::to_string(counts[nodeType]++);
  }

  if (constructorMap.count(nodeType) > 0) {
    return constructorMap[nodeType](assignedName);
  }

  return nullptr;
}