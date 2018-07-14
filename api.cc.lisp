(use-package :cm-ifs)

(with-interface (api)
  (interface-only
   (comment " struct api_t {
  void * (*init)();
  void   (*load)(void *);
  int    (*step)(void *);
  void   (*unload)(void *);
  void   (*deinit)(void *);
};  " :prefix "")))
