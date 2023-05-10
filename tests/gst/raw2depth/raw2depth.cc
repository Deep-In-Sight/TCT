#include <gst/check/gstharness.h>
#include <gtest/gtest.h>
#include <lib/common/tofmeta.h>
#include <lib/raw2depth/raw2depth.h>
#include <math.h>
#include <stdlib.h>

TEST(RawToDepthTestSuite, TestCreateElement) {
  GstHarness *h;

  h = gst_harness_new("raw2depth");

  EXPECT_TRUE(h != NULL) << "cannot create element";

  gst_harness_teardown(h);
}

TEST(RawToDepthTestSuite, TestCapsNegoPass) {
  GstHarness *h;
  GstEvent *e;
  gboolean negotiated;

  GstCaps *src_caps = gst_caps_from_string(
      "video/tof, "
      "format=(string)ek640raw, "
      "pixel_size=(int)2, "
      "num_subframes=(int)4, "
      "width=(int)640, "
      "height=(int)480, "
      "framerate=(fraction)30/1");

  GstCaps *sink_caps = gst_caps_from_string(
      "video/tof, "
      "format=(string)DA_F32");

  h = gst_harness_new("raw2depth");

  EXPECT_TRUE(h != NULL) << "cannot create harness";

  negotiated = FALSE;

  gst_harness_set_caps(h, src_caps, sink_caps);

  while ((e = gst_harness_try_pull_event(h))) {
    if (GST_EVENT_TYPE(e) == GST_EVENT_CAPS) {
      negotiated = TRUE;
    }
    gst_event_unref(e);
  }

  EXPECT_TRUE(negotiated) << "Caps negotiation failed";
}

TEST(RawToDepthTestSuite, TestCapsNegoFailed) {
  GstHarness *h;
  GstEvent *e;
  gboolean negotiated;

  GstCaps *src_caps = gst_caps_from_string(
      "video/tof, "
      "format=(string)ek640raw, "
      "pixel_size=(int)2, "
      "num_subframes=(int)4, "
      "width=(int)640, "
      "height=(int)480, "
      "framerate=(fraction)30/1");

  GstCaps *sink_caps = gst_caps_from_string(
      "video/tof, "
      "format=(string)something");

  h = gst_harness_new("raw2depth");

  EXPECT_TRUE(h != NULL) << "cannot create harness";

  negotiated = FALSE;

  gst_harness_set_caps(h, src_caps, sink_caps);

  while ((e = gst_harness_try_pull_event(h))) {
    if (GST_EVENT_TYPE(e) == GST_EVENT_CAPS) {
      negotiated = TRUE;
    }
    gst_event_unref(e);
  }

  EXPECT_TRUE(!negotiated) << "Caps negotiation not failed";
}

gsize calculate_buf_size(GstCaps *caps) {
  const gchar *fmt;
  gint width, height, pixel_size, num_subframes;
  GstStructure *str = gst_caps_get_structure(caps, 0);
  fmt = gst_structure_get_string(str, "format");
  gst_structure_get_int(str, "width", &width);
  gst_structure_get_int(str, "height", &height);

  if (g_strcmp0(fmt, "ek640raw") == 0) {
    gst_structure_get_int(str, "pixel_size", &pixel_size);
    gst_structure_get_int(str, "num_subframes", &num_subframes);
    return width * height * num_subframes * pixel_size;
  } else if (g_strcmp0(fmt, "DA_F32") == 0) {
    return width * height * sizeof(gfloat) * 2;
  } else if (g_strcmp0(fmt, "D_F32") == 0) {
    return width * height * sizeof(gfloat);
  } else {
    return 0;
  }
}

void prepare_test_data(void *in_data, void *out_data, int size,
                       float mod_freq) {
  gshort *phase, *phase0, *phase2, *phase1, *phase3;
  gfloat *depth;
  gfloat *amplitude;
  float depth_scale;

  /* generate in_data */
  int seed = 123456;
  srand(seed);
  phase = (gshort *)in_data;
  for (int pixel = 0; pixel < size * 4; pixel++) {
    phase[pixel] = rand() % (2 * 32768) - 32768;
  }

  /* generate out_data */
  /* how do we NOT repeating the same code we trying to test over here??? */
  /* using pre-recorded binary data files ? */
  phase0 = (gshort *)in_data;
  phase2 = phase0 + size;
  phase1 = phase2 + size;
  phase3 = phase1 + size;
  depth = (gfloat *)out_data;
  amplitude = depth + size;
  depth_scale = 3e8 / (4 * M_PI) / mod_freq;

  for (int pixel = 0; pixel < size; pixel++) {
    float Q = phase3[pixel] - phase1[pixel];
    float I = phase2[pixel] - phase0[pixel];
    amplitude[pixel] = 0.5 * sqrt(Q * Q + I * I);
    float phase_diff = M_PI + atan2(Q, I);
    depth[pixel] = depth_scale * phase_diff;
  }
}

gboolean compare_data(gchar *data, gchar *expect, gsize size) {
  for (gsize c = 0; c < size; c++) {
    if (data[c] != expect[c]) {
      return FALSE;
    }
  }
  return TRUE;
}

TEST(RawToDepthTestSuite, TestTransformBuffer) {
  GstHarness *h;
  GstBuffer *inbuf = NULL;
  GstBuffer *outbuf = NULL;
  GstBuffer *outbuf_expect = NULL;
  GstMapInfo in_mapinfo, out_mapinfo, out_expect_mapinfo;
  GstFlowReturn ret;
  gsize insize, outsize;
  GstMetaTof *meta;
  GString *src_caps_str;
  GstCaps *src_caps;
  GstCaps *sink_caps;
  gboolean compare_ret;

  int nr_subframes = 4;
  int width = 640;
  int height = 480;

  src_caps_str = g_string_new(NULL);
  g_string_printf(src_caps_str,
                  "video/tof, "
                  "format=(string)ek640raw, "
                  "pixel_size=(int)2, "
                  "num_subframes=(int)%d, "
                  "width=(int)%d, "
                  "height=(int)%d, "
                  "framerate=(fraction)30/1",
                  nr_subframes, width, height);
  src_caps = gst_caps_from_string(src_caps_str->str);
  sink_caps = gst_caps_from_string(
      "video/tof, "
      "format=(string)DA_F32");

  h = gst_harness_new("raw2depth");

  gst_harness_set_caps(h, src_caps, sink_caps);

  insize = calculate_buf_size(src_caps);
  outsize = calculate_buf_size(sink_caps);

  inbuf = gst_harness_create_buffer(h, insize);
  meta = META_TOF_ADD(inbuf);
  meta->modulation_frequency = 24000000;

  outbuf_expect = gst_harness_create_buffer(h, outsize);

  gst_buffer_map(inbuf, &in_mapinfo,
                 (GstMapFlags)(GST_MAP_READ | GST_MAP_WRITE));
  gst_buffer_map(outbuf_expect, &out_expect_mapinfo,
                 (GstMapFlags)(GST_MAP_READ | GST_MAP_WRITE));

  prepare_test_data(in_mapinfo.data, out_expect_mapinfo.data, width * height,
                    meta->modulation_frequency);

  gst_buffer_unmap(inbuf, &in_mapinfo);

  ret = gst_harness_push(h, inbuf);

  EXPECT_TRUE(ret == GST_FLOW_OK) << "cannot push buffer in";

  outbuf = gst_harness_pull(h);

  EXPECT_TRUE(outbuf != NULL) << "did not receive out buffer";

  gst_buffer_map(outbuf, &out_mapinfo, GST_MAP_READ);
  compare_ret = compare_data((gchar *)out_mapinfo.data,
                             (gchar *)out_expect_mapinfo.data, outsize);

  EXPECT_TRUE(compare_ret == TRUE) << "raw to depth calculation failed";

  gst_buffer_unmap(outbuf, &out_mapinfo);
  gst_buffer_unmap(outbuf_expect, &out_expect_mapinfo);

  gst_buffer_unref(outbuf);
  gst_buffer_unref(outbuf_expect);
}

TEST(RawToDepthTestSuite, test_buffer_pool) {
  GstBufferPool *pool;
  GstStructure *config;
  GstBuffer *buffer1, *buffer2;
  GstCaps *some_caps;
  GstFlowReturn ret;

  pool = gst_buffer_pool_new();
  config = gst_buffer_pool_get_config(pool);
  some_caps = gst_caps_from_string("somecaps");

  gst_buffer_pool_config_set_params(config, some_caps, 1024, 1, 0);

  gst_buffer_pool_set_config(pool, config);

  gst_buffer_pool_set_active(pool, TRUE);

  ret = gst_buffer_pool_acquire_buffer(pool, &buffer1, NULL);
  EXPECT_TRUE(ret == GST_FLOW_OK) << " cannot acquire buff";
  gst_buffer_unref(buffer1);

  ret = gst_buffer_pool_acquire_buffer(pool, &buffer2, NULL);
  EXPECT_TRUE(ret == GST_FLOW_OK) << " cannot acquire buff";
  EXPECT_TRUE(buffer1 == buffer2) << "not getting same buffer";
  gst_buffer_unref(buffer2);

  ret = gst_buffer_pool_acquire_buffer(pool, &buffer1, NULL);
  EXPECT_TRUE(ret == GST_FLOW_OK) << " cannot acquire buff";
  ret = gst_buffer_pool_acquire_buffer(pool, &buffer2, NULL);
  EXPECT_TRUE(ret == GST_FLOW_OK) << " cannot acquire buff";
  EXPECT_TRUE(buffer1 != buffer2) << "not getting different buffer";

  gst_buffer_pool_set_active(pool, FALSE);
  gst_caps_unref(some_caps);
  g_object_unref(pool);
}