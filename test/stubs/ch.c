#include <assert.h>

#include "ch.h"

void chDbgAssert(bool assertion, const char *msg) {
  assert(assertion);
}
