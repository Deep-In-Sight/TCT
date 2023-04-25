#include <gst/check/gstcheck.h>
#include <lib/common/tofmeta.h>

GST_START_TEST(test_add_meta) {
  GstBuffer *buffer;
  GstMetaTof *meta_set, *meta_get;

  buffer = gst_buffer_new();

  fail_unless(buffer != NULL, "cannot make buffer");

  meta_set = META_TOF_ADD(buffer);
  fail_unless(meta_set != NULL, "cannot add meta to buffer");

  meta_set->modulation_frequency = 24000000;

  meta_get = META_TOF_GET(buffer);
  fail_unless(meta_get != NULL, "cannot get meta from buffer");
  fail_unless(meta_get->modulation_frequency == 24000000, "wrong meta");

  /* cleanup */
  gst_buffer_unref(buffer);
}
GST_END_TEST;

GST_START_TEST(test_add_meta_params) {
  GstBuffer *buffer;
  GstMetaTof *meta_set, *meta_get;
  guint32 params[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  buffer = gst_buffer_new();

  fail_unless(buffer != NULL, "cannot make buffer");

  meta_set = META_TOF_ADD_PARAMS(buffer, (gpointer)params);
  fail_unless(meta_set != NULL, "cannot add meta to buffer");

  meta_get = META_TOF_GET(buffer);
  fail_unless(meta_get != NULL, "cannot get meta from buffer");
  fail_unless(meta_get->modulation_frequency == 0, "wrong meta");
  fail_unless(meta_get->sensor_temperature == 1, "wrong meta");
  fail_unless(meta_get->rngchk_low == 2, "wrong meta");
  fail_unless(meta_get->rngchk_high == 3, "wrong meta");

  /* cleanup */
  gst_buffer_unref(buffer);
}
GST_END_TEST;

GST_START_TEST(test_add_multi_metas) {
  GstBuffer *buffer;
  GstMetaTof *meta_set, *meta_get;
  guint32 freq = 10;
  guint32 params1[10] = {0, 1, 2, 3, 4, 5, 6};
  guint32 params2[10] = {5, 6, 7, 8, 9, 10, 11};
  gpointer state;

  buffer = gst_buffer_new();

  fail_unless(buffer != NULL, "cannot make buffer");

  // add metas
  meta_set = META_TOF_ADD(buffer);
  fail_unless(meta_set != NULL, "cannot add meta to buffer");
  meta_set->modulation_frequency = freq;

  meta_set = META_TOF_ADD_PARAMS(buffer, params1);
  fail_unless(meta_set != NULL, "cannot add meta to buffer");

  meta_set = META_TOF_ADD_PARAMS(buffer, params2);
  fail_unless(meta_set != NULL, "cannot add meta to buffer");

  // iterate metas
  state = NULL;
  int meta_count = 0;
  while (gst_buffer_iterate_meta(buffer, &state) != NULL) {
    meta_count++;
  }
  fail_unless(meta_count == 3, "not exact number of metas");

  state = NULL;
  meta_count = 0;
  while ((meta_get = (GstMetaTof *)gst_buffer_iterate_meta(buffer, &state))) {
    gboolean ret =
        (meta_count == 0 && meta_get->modulation_frequency == freq) ||
        (meta_count == 1 && meta_get->modulation_frequency == params1[0]) ||
        (meta_count == 2 && meta_get->modulation_frequency == params2[0]);
    fail_unless(ret, "wrong meta");
    meta_count++;
  }

  /* cleanup */
  gst_buffer_unref(buffer);
}
GST_END_TEST;

static Suite *tofmeta_suite(void) {
  Suite *s = suite_create("tofmeta");
  TCase *tc_chain = tcase_create("general");

  suite_add_tcase(s, tc_chain);
  tcase_add_test(tc_chain, test_add_meta);
  tcase_add_test(tc_chain, test_add_meta_params);
  tcase_add_test(tc_chain, test_add_multi_metas);

  return s;
}

GST_CHECK_MAIN(tofmeta);