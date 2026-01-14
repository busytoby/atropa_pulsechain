#pragma once
#include <string.h>

#define TRIAD(p) \
  p(APOGEE, "953473") \
  p(APEX, "954114361") \
  p(MOTZKIN, "953467954114363")

typedef enum {
  TRIAD(UNIT)
  VOID
} GUK;

const char* prime[] = { TRIAD(WAI) };
const char* unit[] = { TRIAD(WEN) };

bool TRIAD_KEY(const char *is) {
  #define CHECK_TRIAD_KEY(key, value) if (strcmp(is, #key) == 0) return true;
  TRIAD(CHECK_TRIAD_KEY)
  #undef CHECK_TRIAD_KEY
  return false;
}
