#include <gst/check/gstcheck.h>
#include <lib/common/tofmeta.h>

GST_START_TEST (test_add_meta)
{
  GstBuffer *buffer;
  GstMetaTof *meta_set, *meta_get;

  buffer = gst_buffer_new_allocate(NULL, 1024, NULL);

  fail_unless(buffer != NULL, "cannot make buffer");

  meta_set = META_TOF_ADD(buffer);
  fail_unless(meta_set != NULL, "cannot add meta to buffer");
  
  meta_set->width = 640;
  meta_set->height = 480;
  meta_set->modulation_frequency = 24000000;

  meta_get = META_TOF_GET(buffer);
  fail_unless(meta_get != NULL, "cannot get meta from buffer");
  fail_unless(meta_get->width == 640, "wrong meta");
  fail_unless(meta_get->height == 480, "wrong meta");
  fail_unless(meta_get->modulation_frequency == 24000000, "wrong meta");
  
  /* cleanup */
  gst_buffer_unref(buffer);
}

GST_END_TEST;

static Suite *
tofmeta_suite (void)
{
  Suite *s = suite_create ("tofmeta");
  TCase *tc_chain = tcase_create ("general");

  suite_add_tcase (s, tc_chain);
  tcase_add_test (tc_chain, test_add_meta);

  return s;
}

GST_CHECK_MAIN (tofmeta);