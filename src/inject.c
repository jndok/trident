#include <stdio.h>
#include "trident/trident.h"

int asd(const char *s)
{
  puts("puts() has been hooked!");
  return puts(s);
}

__attribute__((constructor)) void load(void) {
  hook("_puts", asd);
}
