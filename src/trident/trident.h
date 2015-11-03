/*
*     -= TRIDENT =-
* Mach-O hooking engine.
*
*/

//  Quick note from @jndok:
//  this tool seems to work. changes and updates
//  are planned and are coming soon.
//  will fail on stripped binaries. will also fail if a
//  symbol belonging to the hooked binary image is not found.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>

#define SECT_LAZY_PTRS "__la_symbol_ptr"

typedef struct hook_table {
  void *__symtable;
  void *__strtable;
  void *__indirect_symtable;
  uint32_t __sym_off;
  uint32_t __str_off;
  uint32_t nsyms;
  uint32_t i_undefsyms;
  uint32_t n_undefsyms;
  uint32_t i_indirectsyms;
  uint32_t n_indirectsyms;
  uint32_t lazy_offset;
} hook_table_t;

void *__get_target_base_addr(void *sym);  /* semi-deprecated */
void hook(const char *target, void *replacement, struct mach_header *header_ptr);
