#include <stdlib.h>
#include <glib.h>
#include <gst/gst.h>
#include <gst/gstbuffer.h>
#include <gst/app/gstappsrc.h>
#include <gst/gl/x11/gstgldisplay_x11.h>
#include <gst/gl/gl.h>
#include <gst/app/gstappsink.h>
#include <thread>
#include "record.h"

gboolean jhnn_bus_callback(GstBus *bus, GstMessage *message, struct Vstr_t *v)
{
  g_print("%s\n", GST_MESSAGE_TYPE_NAME(message));
  switch (GST_MESSAGE_TYPE(message))
    {
    case GST_MESSAGE_ERROR:

      GError *err;
      gchar *debug;
      gst_message_parse_error(message, &err, &debug);

      g_error_free(err);
      g_free(debug);
      g_main_loop_quit(v->gloop);
      return TRUE;

    case GST_MESSAGE_EOS:
      printf("eos\n");
      g_main_loop_quit(v->gloop);
      gst_element_set_state(((GstElement*)v->pipeline), GST_STATE_NULL);
      return TRUE;
    }
  return TRUE;
}

GstFlowReturn jhnn_new_preroll(GstAppSink* sink, gpointer data)
{
  g_print("jhnn new preroll");
}

void jhnn_new_eos(GstAppSink* sink, gpointer data)
{
  g_print("jhnn new eos");
}



void sangha_close_pipeline(struct Vstr_t& v) {
gst_element_set_state(((GstElement*)v.pipeline), GST_STATE_NULL);
 g_print("recording loop exited");
  gst_object_unref(v.pipeline);
  gst_object_unref(v.vsrc);
  gst_object_unref(v.fsink);
  g_main_loop_unref(v.gloop);
}


gboolean gstr_step(GMainLoop *loop) {

  return g_main_context_iteration(g_main_loop_get_context(loop), false);

}

void sangha_stop_pipeline(struct Vstr_t& v)
{
g_print("%s\n", gst_flow_get_name(gst_app_src_end_of_stream(v.vsrc)));
gst_element_send_event(v.fsink, gst_event_new_eos());
 while(1) {
   gstr_step(v.gloop);
 }
}

gboolean hydrate_appsrc(GstAppSrc* vsrc, GLubyte* array)
{
int size = 4 * 800 * 800;
GstBuffer* buff = gst_buffer_new_allocate(nullptr, size, nullptr);

gst_buffer_fill(buff, 0, array, size);
return gst_app_src_push_buffer(vsrc, buff);
}

struct Vstr_t sangha_vsrc(const gchar *pipeline_description)
{
  struct Vstr_t v;
  GError *error = NULL;
  if (!gst_is_initialized()) {
    gst_segtrap_set_enabled(FALSE);
    gst_init(NULL, NULL);
  }

  g_print("pipe is: %s", pipeline_description);
  v.pipeline = gst_parse_launch(pipeline_description, &error);
  if (!v.pipeline) {
    g_print("Parse error: %sn", error->message);

  }
  v.vsrc = GST_APP_SRC(gst_bin_get_by_name(GST_BIN(v.pipeline), "vsource"));
  g_assert(v.vsrc);
  v.fsink = GST_ELEMENT(gst_bin_get_by_name(GST_BIN(v.pipeline), "fsink"));
  v.gloop = g_main_loop_new(NULL, FALSE);
  gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(v.pipeline)), ((GstBusFunc)jhnn_bus_callback), &v);

  g_print("%d", gst_element_set_state(((GstElement*)v.pipeline), GST_STATE_PLAYING));
  return v;
}
