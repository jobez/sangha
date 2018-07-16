#ifndef RECORD_INCLUDED
#define RECORD_INCLUDED

#include <GL/glx.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

const char* gstr_pipeline_expr = "appsrc name=vsource format=time is-live=true do-timestamp=true  ! video/x-raw,format=BGRA,width=800,height=800,framerate=0/1 ! videoconvert ! queue ! videoflip method=5  ! queue ! videorate ! queue ! x264enc bitrate=32768 dct8x8=true  ! video/x-h264,profile=baseline,framerate=24/1 ! queue ! mp4mux name=mux  ! filesink name=fsink location=./exemplo.mp4 ";

/* struct Vstr_t */
/* { */
/*   GstElement *pipeline; */
/*   GstElement *fsink; */
/*   GstAppSrc *vsrc; */
/*   GMainLoop *gloop; */
/* }; */


/* void sangha_stop_gstr(struct Vstr_t& v) {} */

/* gboolean hydrate_appsrc(GstAppSrc* vsrc, GLubyte* array) {} */

/* void sangha_stop_pipeline(struct Vstr_t& v) {} */

/* void sangha_close_pipeline(struct Vstr_t& v) {} */

/* gboolean gstr_step(GMainLoop *loop) {} */

/* struct Vstr_t sangha_vsrc(const gchar *pipeline_description) {} */

#endif
