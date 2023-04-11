#include <gst/check/gstcheck.h>
#include <gst/check/gstharness.h>
#include <lib/common/tofmeta.h>
#include <lib/raw2depth/raw2depth.h>

GST_START_TEST(test_create_element) {
  GstHarness *h;

  h = gst_harness_new("raw2depth");

  fail_unless(h != NULL, "cannot create element");

  gst_harness_teardown(h);
}
GST_END_TEST;

GST_START_TEST(test_caps_nego_pass) {
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

  fail_unless(h != NULL, "cannot create harness");

  negotiated = FALSE;

  gst_harness_set_caps(h, src_caps, sink_caps);

  while ((e = gst_harness_try_pull_event(h))) {
    if (GST_EVENT_TYPE(e) == GST_EVENT_CAPS) {
      negotiated = TRUE;
    }
    gst_event_unref(e);
  }

  fail_unless(negotiated, "Caps negotiation failed");
}
GST_END_TEST;

GST_START_TEST(test_caps_nego_fail) {
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

  fail_unless(h != NULL, "cannot create harness");

  negotiated = FALSE;

  gst_harness_set_caps(h, src_caps, sink_caps);

  while ((e = gst_harness_try_pull_event(h))) {
    if (GST_EVENT_TYPE(e) == GST_EVENT_CAPS) {
      negotiated = TRUE;
    }
    gst_event_unref(e);
  }

  fail_unless(!negotiated, "Caps negotiation not failed");
}
GST_END_TEST;

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

GST_START_TEST(test_transform_buffer) {
  GstHarness *h;
  GstBuffer *inbuf = NULL;
  GstBuffer *outbuf1 = NULL;
  GstBuffer *outbuf2 = NULL;
  GstFlowReturn ret;
  gsize insize, outsize;

  GstCaps *src_caps = gst_caps_from_string(
      "video/tof, "
      "format=(string)ek640raw, "
      "pixel_size=(int)2, "
      "num_subframes=(int)4, "
      "width=(int)10, "
      "height=(int)4, "
      "framerate=(fraction)30/1");

  GstCaps *sink_caps = gst_caps_from_string(
      "video/tof, "
      "format=(string)DA_F32");

  h = gst_harness_new("raw2depth");

  gst_harness_set_caps(h, src_caps, sink_caps);

  insize = calculate_buf_size(src_caps);
  outsize = calculate_buf_size(sink_caps);

  inbuf = gst_harness_create_buffer(h, insize);
  ret = gst_harness_push(h, inbuf);

  fail_unless(ret == GST_FLOW_OK, "cannot push buffer in");

  outbuf1 = gst_harness_pull(h);

  fail_unless(outbuf1 != NULL, "did not receive out buffer");
  fail_unless(gst_buffer_get_size(outbuf1) == outsize,
              "wrong output buffer size");

  gst_buffer_unref(outbuf1);

  inbuf = gst_harness_create_buffer(h, insize);
  ret = gst_harness_push(h, inbuf);
  outbuf2 = gst_harness_pull(h);

  fail_unless(outbuf2 != NULL, "cannot pull outbuf2");

  fail_unless(outbuf1 == outbuf2, "didn't reuse buffer");

  gst_buffer_unref(outbuf2);
}
GST_END_TEST;

GST_START_TEST(test_buffer_pool) {
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
  fail_unless(ret == GST_FLOW_OK, " cannot acquire buff");
  gst_buffer_unref(buffer1);

  ret = gst_buffer_pool_acquire_buffer(pool, &buffer2, NULL);
  fail_unless(ret == GST_FLOW_OK, " cannot acquire buff");
  fail_unless(buffer1 == buffer2, "not getting same buffer");
  gst_buffer_unref(buffer2);

  ret = gst_buffer_pool_acquire_buffer(pool, &buffer1, NULL);
  fail_unless(ret == GST_FLOW_OK, " cannot acquire buff");
  ret = gst_buffer_pool_acquire_buffer(pool, &buffer2, NULL);
  fail_unless(ret == GST_FLOW_OK, " cannot acquire buff");
  fail_unless(buffer1 != buffer2, "not getting different buffer");

  gst_buffer_pool_set_active(pool, FALSE);
  gst_caps_unref(some_caps);
  g_object_unref(pool);
}
GST_END_TEST;

static Suite *raw2depth_suite(void) {
  Suite *s = suite_create("raw2depth");
  TCase *tc_chain = tcase_create("general");

  suite_add_tcase(s, tc_chain);
  tcase_add_test(tc_chain, test_create_element);
  /* TODO: add (supposedly) fail test cases */
  tcase_add_test(tc_chain, test_caps_nego_pass);
  tcase_add_test(tc_chain, test_caps_nego_fail);
  tcase_add_test(tc_chain, test_transform_buffer);
  tcase_add_test(tc_chain, test_buffer_pool);

  return s;
}

GST_CHECK_MAIN(raw2depth);