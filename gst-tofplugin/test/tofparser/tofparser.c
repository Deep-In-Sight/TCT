#include <gst/check/gstcheck.h>
#include <gst/check/gstharness.h>
#include <lib/common/tofmeta.h>
#include <lib/tofparser/tofparser.h>
#include <string.h>

#define FILENAME "testdata"

StreamHeader aheader = {.container_header_size = 1024,
                        .subframe_header_size = 1024,
                        .frame_width = 640,
                        .frame_height = 480,
                        .framerate_num = 30,
                        .framerate_den = 1,
                        .pixel_size = 2,
                        .num_subframes = 4,
                        .num_frames = 30};

static guint32 sf_header[6] = {1, 2, 3, 4, 5, 6};

void *raw_data;
gsize bs = 4096;
gsize raw_data_size;
gsize sf_payload_size;
gsize sf_size, df_size;

void setup_data(void) {
  guint8 *raw_data_cur;

  sf_payload_size =
      aheader.frame_width * aheader.frame_height * aheader.pixel_size;
  sf_size = (aheader.subframe_header_size + sf_payload_size);
  df_size = aheader.num_subframes * sf_size;
  raw_data_size = aheader.container_header_size + aheader.num_frames * df_size;

  raw_data = g_malloc(raw_data_size);
  raw_data_cur = (guint8 *)raw_data;

  memcpy(raw_data_cur, &aheader, sizeof(aheader));
  raw_data_cur += aheader.container_header_size;  // jump to first subframe

  for (int df = 0; df < aheader.num_frames; df++) {
    for (int sf = 0; sf < aheader.num_subframes; sf++) {
      memcpy(raw_data_cur, sf_header, sizeof(sf_header));
      raw_data_cur += sf_size;  // jump to next subframe
    }
  }
}

void cleanup_data(void) { g_free(raw_data); }

// return true if correct
gboolean meta_check(GstMetaTof *meta) {
  return (meta->modulation_frequency == sf_header[0]) &&
         (meta->sensor_temperature == sf_header[1]) &&
         (meta->rngchk_low == sf_header[2]) &&
         (meta->rngchk_high == sf_header[3]);
}

// split data into blocksize buffers, and send to harness
void feed_buffer(GstHarness *h, guint8 *data, gsize datasize, gsize blocksize) {
  GstBuffer *inbuf;
  gsize buffer_offset, buffer_size;
  GstSegment segment;

  gsize num_buffer = datasize / blocksize;
  if (num_buffer * blocksize < datasize) {
    num_buffer++;
  }

  gst_segment_init(&segment, GST_FORMAT_TIME);

  fail_unless(gst_harness_push_event(h, gst_event_new_segment(&segment)),
              "cannot push segment event");

  for (int block = 0; block < num_buffer; block++) {
    buffer_offset = block * blocksize;
    if (buffer_offset + blocksize < datasize) {
      buffer_size = blocksize;
    } else {
      buffer_size = datasize - buffer_offset;
    }
    inbuf = gst_buffer_new_memdup(data + buffer_offset, buffer_size);
    fail_unless(gst_harness_push(h, inbuf) == GST_FLOW_OK,
                "push buffer failed");
  }

  fail_unless(gst_harness_push_event(h, gst_event_new_eos()),
              "cannot end stream");
}

GST_START_TEST(test_caps) {
  GstHarness *h;
  gsize blocksize = bs;

  setup_data();

  h = gst_harness_new("tofparser");
  fail_unless(h != NULL, "cannot create harness");

  GString *caps_str = g_string_new(NULL);
  g_string_printf(
      caps_str,
      "video/tof, format=(string)ek640raw, width=(int)%d, height=(int)%d, "
      "pixel_size=(int)%d, num_subframes=(int)%d, framerate=(fraction)%d/%d",
      aheader.frame_width, aheader.frame_height, aheader.pixel_size,
      aheader.num_subframes, aheader.framerate_num, aheader.framerate_den);

  gst_harness_set_sink_caps_str(h, caps_str->str);

  feed_buffer(h, raw_data, raw_data_size, blocksize);

  fail_unless(gst_harness_buffers_in_queue(h) == aheader.num_frames,
              "received wrong number of frames");

  gst_harness_teardown(h);
  g_string_free(caps_str, TRUE);

  cleanup_data();
}
GST_END_TEST;

GST_START_TEST(test_frame_count) {
  GstHarness *h;
  gsize blocksize = bs;

  setup_data();

  h = gst_harness_new("tofparser");
  fail_unless(h != NULL, "cannot create harness");

  feed_buffer(h, raw_data, raw_data_size, blocksize);

  fail_unless(gst_harness_buffers_in_queue(h) == aheader.num_frames,
              "received wrong number of frames");

  gst_harness_teardown(h);

  cleanup_data();
}
GST_END_TEST;

GST_START_TEST(test_frame_meta) {
  GstHarness *h;
  GstBuffer *outbuf;
  gsize blocksize = bs;
  gsize outbuf_size, expected_size;
  GstMetaTof *meta;
  gpointer state;
  int meta_count;

  setup_data();

  h = gst_harness_new("tofparser");
  fail_unless(h != NULL, "cannot create harness");

  feed_buffer(h, raw_data, raw_data_size, blocksize);

  expected_size = sf_payload_size * aheader.num_subframes;

  for (int df = 0; df < aheader.num_frames; df++) {
    outbuf = gst_harness_pull(h);
    fail_unless(outbuf != NULL, "cannot pull buffer out");
    outbuf_size = gst_buffer_get_size(outbuf);
    fail_unless(outbuf_size == expected_size,
                "frame received has incorrect size");
    meta_count = 0;
    state = NULL;
    while ((meta = (GstMetaTof *)gst_buffer_iterate_meta(outbuf, &state))) {
      fail_unless(meta_check(meta), "framebuffer has wrong meta value");
      meta_count++;
    }
    fail_unless(meta_count == aheader.num_subframes,
                "frame buffer has wrong meta count");
  }

  gst_buffer_unref(outbuf);
  gst_harness_teardown(h);

  cleanup_data();
}

GST_END_TEST;

static Suite *tofparser_suite(void) {
  Suite *s = suite_create("tofparser");
  TCase *tc_chain = tcase_create("general");

  suite_add_tcase(s, tc_chain);
  tcase_add_test(tc_chain, test_caps);
  tcase_add_test(tc_chain, test_frame_count);
  tcase_add_test(tc_chain, test_frame_meta);

  return s;
}

GST_CHECK_MAIN(tofparser);