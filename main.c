#define _BSD_SOURCE // usleep()
#include <unistd.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include "api.h"
#include <iostream>
#include <thread>


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



void enact_engine_loop(const char* shared_object_path, struct app_t engine) {
  while(1) {
    AppLoad(shared_object_path, &engine);

    if (engine.handle != NULL)
      if (engine.api.Step(engine.state) != 0)
        break;
  }
  AppUnload(&engine);
}

int main() {
  struct app_t vengine = {0};
  struct app_t aengine = {0};
  /* std::thread visual_engine_thread(enact_engine_loop, "./libaengine.so", aengine); */
  /* std::thread audio_engine_thread(enact_engine_loop, "./libaengine.so", aengine); */
  /* audio_engine_thread.join(); */
  /* visual_engine_thread.join(); */

  while(1) {
    AppLoad("./libaengine.so", &aengine);
    AppLoad("./libvengine.so", &vengine);

    if (aengine.handle != NULL)
      if (aengine.api.Step(aengine.state) != 0)
        break;

    if (vengine.handle != NULL)
      if (vengine.api.Step2(vengine.state, aengine.state) != 0)
        break;

  }
  AppUnload(&vengine);
  AppUnload(&aengine);

  /* enact_engine("./libvengine.so", vengine); */


  /* AppLoad("./libaengine.so", &aengine); */


  return 0;
}
