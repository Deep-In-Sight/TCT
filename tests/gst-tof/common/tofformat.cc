#include <gst-tof/common/tofformat.h>
#include <gtest/gtest.h>

TEST(TofFormatTestSuite, TestRawFormats) {
  GValue *raw_fmts = tofformat_get_raw_fmts();
  gsize num_fmts = tofformat_get_num_raw_fmts();
  EXPECT_TRUE(raw_fmts != NULL) << "no raw formats";
  EXPECT_TRUE(G_VALUE_TYPE(raw_fmts) == GST_TYPE_LIST) << "not a list";
  EXPECT_TRUE(gst_value_list_get_size(raw_fmts) == num_fmts)
      << "wrong number of formats";
  const GValue *fmt = gst_value_list_get_value(raw_fmts, 0);
  EXPECT_TRUE(g_strcmp0("ek640raw", g_value_get_string(fmt)) == 0)
      << "ek640raw not supported";
}

TEST(TofFormatTestSuite, TestDepthFormats) {
  GValue *depth_fmts = tofformat_get_depth_fmts();
  gsize num_fmts = tofformat_get_num_depth_fmts();
  EXPECT_TRUE(depth_fmts != NULL) << "no depth formats";
  EXPECT_TRUE(G_VALUE_TYPE(depth_fmts) == GST_TYPE_LIST) << "not a list";
  EXPECT_TRUE(gst_value_list_get_size(depth_fmts) == num_fmts)
      << "wrong number of formats";
  const GValue *fmt = gst_value_list_get_value(depth_fmts, 0);
  EXPECT_TRUE(g_strcmp0("D_F32", g_value_get_string(fmt)) == 0)
      << "D_F32 not supported";
}