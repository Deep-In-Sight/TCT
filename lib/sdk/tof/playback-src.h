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
#ifndef __PLAYBACK_SRC_H__
#define __PLAYBACK_SRC_H__

#include <sdk/core/base-src.h>

class PlaybackSource : public BaseSource {
 public:
  PlaybackSource(const string& name, bool is_async, bool loop = false);
  ~PlaybackSource();

  void SetFilename(const string& filename);
  /**
   * @brief Manually set the format of the source. To be removed later and parse
   * file header instead.
   *
   * @param size
   * @param type
   */
  void SetFormat(const MatShape& shape, int type);

  void SetFrameRate(float fps);

  void SetLoop(bool loop);

  bool InitializeSource() override;
  Mat GenerateFrame() override;
  void CleanupSource() override;

 private:
  string filename_;
  FILE* file_;
  bool loop_;
  MatShape shape_;
  int type_;
  float frame_duration_;
  float sleep_duration_ms_;
  chrono::time_point<chrono::steady_clock> last_frame_time_;
};

#endif  // __PLAYBACK_SRC_H__