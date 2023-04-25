#include <gst/check/check.h>
#include <lib/common/tofformat.h>

GST_START_TEST(test_raw_formats) {
  GValue *raw_fmts = tofformat_get_raw_fmts();
  gsize num_fmts = tofformat_get_num_raw_fmts();
  fail_unless(raw_fmts != NULL, "no raw formats");
  fail_unless(G_VALUE_TYPE(raw_fmts) == GST_TYPE_LIST, "not a list");
  fail_unless(gst_value_list_get_size(raw_fmts) == num_fmts,
              "more than %d formats", num_fmts);
  const GValue *fmt = gst_value_list_get_value(raw_fmts, 0);
  fail_unless(g_strcmp0("ek640raw", g_value_get_string(fmt)) == 0,
              "ek640raw not supported");
}
GST_END_TEST;

GST_START_TEST(test_depth_formats) {
  GValue *depth_fmts = tofformat_get_depth_fmts();
  gsize num_fmts = tofformat_get_num_depth_fmts();
  fail_unless(depth_fmts != NULL, "no depth formats");
  fail_unless(G_VALUE_TYPE(depth_fmts) == GST_TYPE_LIST, "not a list");
  fail_unless(gst_value_list_get_size(depth_fmts) == num_fmts,
              "more than %d formats", num_fmts);
  const GValue *fmt = gst_value_list_get_value(depth_fmts, 0);
  fail_unless(g_strcmp0("D_F32", g_value_get_string(fmt)) == 0,
              "D_F32 not supported");
}
GST_END_TEST;

static Suite *tofformat_suite(void) {
  Suite *s = suite_create("tofformat");
  TCase *tc_chain = tcase_create("general");

  suite_add_tcase(s, tc_chain);
  tcase_add_test(tc_chain, test_raw_formats);
  tcase_add_test(tc_chain, test_depth_formats);

  return s;
}

GST_CHECK_MAIN(tofformat);