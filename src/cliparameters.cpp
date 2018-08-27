#include "include/types.hpp"
#include "include/cliparameters.hpp"
#include <cstring>

const char* CLIParameters::get(const char* name, const char* def) const {
  int argMax = argCount - 1;
  for (int i = 0; i < argMax; i++) {
    if (i < argCount && !strcmp(name, argList[i])) {
      return argList[i + 1];
    }
  }
  return def;
}

bool CLIParameters::has(const char* name) const {
  for (int i = 0; i < argCount; i++) {
    if (!strcmp(name, argList[i])) {
      return true;
    }
  }
  return false;
}
