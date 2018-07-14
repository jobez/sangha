;; -*- mode: Lisp; eval: (cm-mode 1); -*-
(use-package :cm-ifs)

(include <sys/mman.h>)
(include <stdio.h>)
(include "api.h")

(struct state-t)
(decl ((state-t* s = NULL))

  (function app-init ()
      -> (static void*)
    (decl ((void* state = (mmap 0 (cast long (* 256
                                                1024
                                                1024
                                                1024))
                                (\| PROT_READ PROT_WRITE)
                                (\| MAP_ANONYMOUS MAP_PRIVATE
                                    MAP_NORESERVE)
                                -1
                                0)))
      (set s state)

      (printf "init")
      (return state)
      ))


  (function app-load ((void* state))
      -> (static void)
    (set s state)
    (printf "reload")
    )


  (function app-step ((void* state))
      -> (static int)
    (set s state)
    (printf "step")
    (return 0))

  (function app-unload ((void* state))
      -> (static void)
    (set s state)
    (printf "unload")
    )

  (function app-deinit ((void* state))
      -> (static void)
    (set s state)
    (printf "finalize")
    (munmap state (cast long (* 256
                                1024
                                1024
                                1024))))






  (comment "struct api_t APP_API = {
  .init   = app_init,
  .load   = app_load,
  .step   = app_step,
  .unload = app_unload,
  .deinit = app_deinit
}; " :prefix "")



  )
