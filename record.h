#pragma once

#include <GL/glx.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

struct Vstr_t
{
  GstElement *pipeline;
  GstElement *fsink;
  GstAppSrc *vsrc;
  GMainLoop *gloop;
};


void sangha_stop_gstr(struct Vstr_t& v);

gboolean hydrate_appsrc(GstAppSrc* vsrc, GLubyte* array);

void sangha_stop_pipeline(struct Vstr_t& v);

void sangha_close_pipeline(struct Vstr_t& v);

gboolean gstr_step(GMainLoop *loop);

struct Vstr_t sangha_vsrc(const gchar *pipeline_description);
