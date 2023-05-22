/* Copyright (C) 2023 Deep In Sight
 * Author: Le Ngoc Linh <lnlinh93@dinsight.ai>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION: inspector
 *
 * Add description later
 *
 */
#include <sdk/inspector/inspector-queue.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>

using namespace spdlog;
static logger* logger_ = stdout_color_mt("InspectorQueue").get();

InspectorQueue::InspectorQueue(const std::string name) : name_(name) {}

InspectorQueue::~InspectorQueue() {}

void InspectorQueue::AddInspector(PadObserver* inspector) {
  Pad* pad = queue_.GetSourcePad();
  pad->AddObserver(inspector);
}

void InspectorQueue::RemoveInspector(PadObserver* inspector) {
  Pad* pad = queue_.GetSourcePad();
  pad->RemoveObserver(inspector);
}

void InspectorQueue::OnNewFrame(cv::Mat& frame) {
  // avoid wasting queue memory if no observer
  Pad* pad = queue_.GetSourcePad();
  if (pad->GetObserverCount() == 0) {
    return;
  }
  queue_.PushFrame(frame);
}

void InspectorQueue::SetSizeType(Size size, int type) {
  // notify other observers
  queue_.SetSizeType(size, type);
}