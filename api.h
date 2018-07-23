#pragma once

struct api_t {
  void * (*Init)();
  void   (*Load)(void *);
  int    (*Step)(void *);
  int    (*Step2)(void *, void *);
  void   (*Unload)(void *);
  void   (*Deinit)(void *);
};
