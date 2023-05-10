#include <gtest/gtest.h>
#include <lib/common/tofmeta.h>

TEST(TofMetaTestSuite, TestAddMeta) {
  GstBuffer *buffer;
  GstMetaTof *meta_set, *meta_get;

  buffer = gst_buffer_new();

  EXPECT_TRUE(buffer != NULL) << "cannot make buffer" << __FILE__;

  meta_set = META_TOF_ADD(buffer);
  EXPECT_TRUE(meta_set != NULL) << "cannot add meta to buffer";

  meta_set->modulation_frequency = 24000000;

  meta_get = META_TOF_GET(buffer);
  EXPECT_TRUE(meta_get != NULL) << "cannot get meta from buffer";
  EXPECT_TRUE(meta_get->modulation_frequency == 24000000) << "wrong meta";

  /* cleanup */
  gst_buffer_unref(buffer);
}

TEST(TofMetaTestSuite, TestAddMetaWithParam) {
  GstBuffer *buffer;
  GstMetaTof *meta_set, *meta_get;
  guint32 params[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  buffer = gst_buffer_new();

  EXPECT_TRUE(buffer != NULL) << "cannot make buffer";

  meta_set = META_TOF_ADD_PARAMS(buffer, (gpointer)params);
  EXPECT_TRUE(meta_set != NULL) << "cannot add meta to buffer";

  meta_get = META_TOF_GET(buffer);
  EXPECT_TRUE(meta_get != NULL) << "cannot get meta from buffer";
  EXPECT_TRUE(meta_get->modulation_frequency == 0) << "wrong meta";
  EXPECT_TRUE(meta_get->sensor_temperature == 1) << "wrong meta";
  EXPECT_TRUE(meta_get->rngchk_low == 2) << "wrong meta";
  EXPECT_TRUE(meta_get->rngchk_high == 3) << "wrong meta";

  /* cleanup */
  gst_buffer_unref(buffer);
}

TEST(TofMetaTestSuite, TestAddMultiMeta) {
  GstBuffer *buffer;
  GstMetaTof *meta_set, *meta_get;
  guint32 freq = 10;
  guint32 params1[10] = {0, 1, 2, 3, 4, 5, 6};
  guint32 params2[10] = {5, 6, 7, 8, 9, 10, 11};
  gpointer state;

  buffer = gst_buffer_new();

  EXPECT_TRUE(buffer != NULL) << "cannot make buffer";

  // add metas
  meta_set = META_TOF_ADD(buffer);
  EXPECT_TRUE(meta_set != NULL) << "cannot add meta to buffer";
  meta_set->modulation_frequency = freq;

  meta_set = META_TOF_ADD_PARAMS(buffer, params1);
  EXPECT_TRUE(meta_set != NULL) << "cannot add meta to buffer";

  meta_set = META_TOF_ADD_PARAMS(buffer, params2);
  EXPECT_TRUE(meta_set != NULL) << "cannot add meta to buffer";

  // iterate metas
  state = NULL;
  int meta_count = 0;
  while (gst_buffer_iterate_meta(buffer, &state) != NULL) {
    meta_count++;
  }
  EXPECT_TRUE(meta_count == 3) << "not exact number of metas";

  state = NULL;
  meta_count = 0;
  while ((meta_get = (GstMetaTof *)gst_buffer_iterate_meta(buffer, &state))) {
    gboolean ret =
        (meta_count == 0 && meta_get->modulation_frequency == freq) ||
        (meta_count == 1 && meta_get->modulation_frequency == params1[0]) ||
        (meta_count == 2 && meta_get->modulation_frequency == params2[0]);
    EXPECT_TRUE(ret) << "wrong meta";
    meta_count++;
  }

  /* cleanup */
  gst_buffer_unref(buffer);
}