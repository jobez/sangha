#define _BSD_SOURCE // usleep()
#include <unistd.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include "api.h"
#include <iostream>

struct app_t {
  struct api_t api;
  void * handle;
  void * state;
  ino_t  id;
};

static void AppLoad(const char* shared_object_path, struct app_t * app) {

  struct stat attr = {0};
  if ((stat(shared_object_path, &attr) == 0) && (app->id != attr.st_ino)) {
    if (app->handle != NULL) {
      app->api.Unload(app->state);
      dlclose(app->handle);
    }
    void * handle = dlopen(shared_object_path, RTLD_NOW | RTLD_GLOBAL);
    if (handle != NULL) {
      std::cout << "hey" << std::endl;
      app->handle = handle;
      app->id = attr.st_ino;
      struct api_t * api = (api_t*)dlsym(app->handle, "APP_API");
      if (api != NULL) {
        app->api = api[0];
        if (app->state == NULL)
          app->state = app->api.Init();
        app->api.Load(app->state);
      } else {
        dlclose(app->handle);
        app->handle = NULL;
        app->id = 0;
      }
    } else {
      fprintf(stderr, "dlopen failed: %s\n", dlerror());

      app->handle = NULL;
      app->id = 0;
    }
  }
}

void AppUnload(struct app_t * app) {
  if (app->handle != NULL) {
    app->api.Unload(app->state);
    app->api.Deinit(app->state);
    app->state = NULL;
    dlclose(app->handle);
    app->handle = NULL;
    app->id = 0;
  }
}

int main() {
  struct app_t renderer = {0};
  while(1) {
    AppLoad("./librenderer.so", &renderer);
    if (renderer.handle != NULL)
      if (renderer.api.Step(renderer.state) != 0)
        break;
  }
  AppUnload(&renderer);
  return 0;
}
