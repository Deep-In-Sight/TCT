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
  float fps_;
};

#endif  // __PLAYBACK_SRC_H__