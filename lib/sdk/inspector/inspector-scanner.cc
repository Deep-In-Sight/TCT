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

#include <sdk/inspector/inspector-scanner.h>

void InspectorScanner::GetRange(int& x1, int& y1, int& xy2) {
  x1 = start_x;
  y1 = start_y;
  xy2 = end_xy;
}

void InspectorScanner::Update(GstBuffer* buffer) {
  /* well.. argument validation and return check be damned */
  auto vec = CollectRange(buffer);
  RenderResult(vec);
}

void InspectorScanner::SetRangeImpl(int x1, int y1, int xy2,
                                    ScanDirection dir) {
  size_t vec_size;

  start_x = std::min(width - 1, std::max(0, x1));
  start_y = std::min(height - 1, std::max(0, y1));
  if (dir == kScanHorizontal) {
    end_xy = std::min(width - 1, std::max(0, xy2));
    if (end_xy < start_x) {
      std::swap(start_x, end_xy);
    }
    vec_size = end_xy - start_x;
  } else {
    end_xy = std::min(height - 1, std::max(0, xy2));
    if (end_xy < start_y) {
      std::swap(start_y, end_xy);
    }
    vec_size = end_xy - start_y;
  }

  if (vec_size > collected.capacity()) {
    collected.reserve(vec_size);
  }
}

const std::vector<float>& InspectorScanner::CollectRangeImpl(
    GstBuffer* buffer, ScanDirection dir) {
  GstMapInfo mapinfo;
  float* data;
  size_t stride;
  int start, end;

  gst_buffer_map(buffer, &mapinfo, GST_MAP_READ);
  data = reinterpret_cast<float*>(mapinfo.data);
  if (format == "DA_F32" && is_amplitude) {
    data += width * height;
  }
  data += (width * start_y + start_x);
  if (dir == kScanHorizontal) {
    stride = 1;
    start = start_x;
    end = end_xy;
  } else {
    stride = width;
    start = start_y;
    end = end_xy;
  }

  collected.clear();

  for (int i = 0; i <= end - start; i++) {
    collected.push_back(data[i * stride]);
  }

  gst_buffer_unmap(buffer, &mapinfo);

  return collected;
}

void InspectorHScanner::SetRange(int x1, int y1, int xy2) {
  SetRangeImpl(x1, y1, xy2, InspectorScanner::kScanHorizontal);
}

const std::vector<float>& InspectorHScanner::CollectRange(GstBuffer* buffer) {
  return CollectRangeImpl(buffer, InspectorScanner::kScanHorizontal);
}

void InspectorVScanner::SetRange(int x1, int y1, int xy2) {
  SetRangeImpl(x1, y1, xy2, InspectorScanner::kScanVertical);
}

const std::vector<float>& InspectorVScanner::CollectRange(GstBuffer* buffer) {
  return CollectRangeImpl(buffer, InspectorScanner::kScanVertical);
}