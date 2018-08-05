#pragma once

#include <GL/glx.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

typedef struct
{
  GstElement *pipeline;
  GstElement *fsink;
  GstAppSrc *vsrc;
  GMainLoop *gloop;
} Vstr_t;


void sangha_stop_gstr(Vstr_t& v);

gboolean hydrate_appsrc(GstAppSrc* vsrc, GLubyte* array);

void sangha_stop_pipeline(Vstr_t& v);

void sangha_close_pipeline(Vstr_t& v);

gboolean gstr_step(GMainLoop *loop);

Vstr_t sangha_vsrc(const gchar *pipeline_description);
