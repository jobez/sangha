;; (use-package :cm-ifs)
(include <stdio.h>)
(include <unistd.h>)
(include <sys/stat.h>)
(include <dlfcn.h>)
(include "api.h")

(typedef struct api-t api-t)

(struct app-t
  (public
   (api-t api)
   (void* handle)
   (void* state)
   (ino-t id)))

(function app-load ((struct app-t* app))
    -> (static void)
  (decl ((struct stat attr)))
  (set attr (clist 0))
  (when (&& (== (stat "./libapp.so" &attr) 0)
            (!= app->id attr.st_ino))

    (when (!= app->handle
              NULL)
      (app->api.unload app->state)
      (dlclose app->handle))
    (decl ((void* handle = (dlopen "./libapp.so" RTLD_NOW)))
      (if (!= handle NULL)
          (progn
            (set app->handle handle)
            (set app->id attr.st_ino)
            (decl ((struct api-t* api = (cast api_t* (dlsym app->handle "APP_API"))))
              (if (!= api NULL)
                  (progn
                    (printf "api dlsym :)")
                    (set app->api api[0])
                    (when (== app->state NULL)
                      (set app->state (app->api.init))))
                  (progn
                    (printf "api dlsym :(")
                    (dlclose app->handle)
                    (set app->handle NULL)
                    (set app->id 0))
                  )))
          (progn
            (set app->handle NULL)
            (set app->id 0))))))

(function app-unload ((struct app-t* app))
    -> (static void)
  (when (!= app->handle NULL)
    (app->api.unload app->state)
    (app->api.deinit app->state)
    (set app->state NULL)
    (dlclose app->handle)
    (set app->handle NULL)
    (set app->id 0)))

(function main () -> int
  (decl ((app-t app))

    (set app (clist 0))

    (for ((int i = 0)
          (< i 1000)
          (+= i 1))
      (app-load &app)
      (when (!= app.handle NULL)
        (printf "sup\\n")
        (when (!= (app.api.step app.state)
                  0)
          (break)))

      (usleep 10000)
      )
    (app-unload &app)
    (return 0)))
