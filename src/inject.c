#include <stdio.h>
#include "trident/trident.h"

struct ProgramVars {
    struct mach_header*	mh;
    int*		NXArgcPtr;
    const char***	NXArgvPtr;
    const char***	environPtr;
    const char**	__prognamePtr;
};

int test(const char *s)
{
  puts("puts() has been hooked!");
  return puts(s);
}

__attribute__((constructor)) void load(int argc, const char **argv, const char **envp, const char **apple, struct ProgramVars *pvars) {
  hook("_puts", test, pvars->mh);
}
