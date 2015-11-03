/*
this is a test file. it actually works better than trident,
as it hooks more files! (trident crashes on some files,
probably due to bad mach-o parsing)

this will be integrated soon-ish, but I am pushing it
with the project for now.
*/

#include <stdio.h>
#include <string.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>

struct ProgramVars {
    struct mach_header*	mh;
    int*		NXArgcPtr;
    const char***	NXArgvPtr;
    const char***	environPtr;
    const char**	__prognamePtr;
};

void kek (void) {
  printf("== hooked ==\n");
}

__attribute__((constructor)) void load(int argc, const char **argv, const char **envp, const char **apple, struct ProgramVars *pvars) {
  printf("[+] Injected! mach_heder found @ %p\n", pvars->mh);
  void *base=pvars->mh;
  void *__symtable=NULL;
  void *__strtable=NULL;

  uint32_t nsyms=0;
  uint32_t indirectsymoff; /* file offset to the indirect symbol table */
  uint32_t nindirectsyms;  /* number of indirect symbol table entries */
  uint32_t iundefsym;	/* index to undefined symbols */
  uint32_t nundefsym;	/* number of undefined symbols */
  uint32_t stuff=0;

  struct load_command *lcmds = base+sizeof(struct mach_header_64);
  for (uint32_t i = 0; i < ((struct mach_header_64*)(base))->ncmds; i++) {
    if (lcmds->cmd == LC_SEGMENT_64) {
      struct segment_command_64 *seg = (struct segment_command_64*)lcmds;
      if (strcmp(seg->segname, SEG_TEXT) == 0) {
        puts("text found!");
      } else if (strcmp(seg->segname, SEG_DATA) == 0) {
        struct section_64 *sec = (struct section_64*)((void*)seg+sizeof(struct segment_command_64));
        for (uint32_t i = 0; i < seg->nsects; i++) {
          if (strcmp(sec->sectname, "__la_symbol_ptr") == 0) {
            stuff=sec->offset;
          }
          sec = ((void*)sec+sizeof(struct section_64));
        }
      }
    } else if (lcmds->cmd == LC_SYMTAB) {
      struct symtab_command *sym_cmd = (struct symtab_command*)lcmds;
      puts("[+] LC_SYMTAB found!");
      printf("symtable offset: %x\nnsyms: %d\n", sym_cmd->symoff, sym_cmd->nsyms);
      __symtable = base + sym_cmd->symoff;
      __strtable = base + sym_cmd->stroff;
      nsyms=sym_cmd->nsyms;
    } else if (lcmds->cmd == LC_DYSYMTAB) {
      struct dysymtab_command *dysym_cmd = (struct dysymtab_command*)lcmds;
      puts("[+] LC_DYSYMTAB found!");
      indirectsymoff = dysym_cmd->indirectsymoff;
      nindirectsyms = dysym_cmd->nindirectsyms;
      iundefsym = dysym_cmd->iundefsym;
      nundefsym = dysym_cmd->nundefsym;
    }

    lcmds = ((void*)lcmds + lcmds->cmdsize);
  }

  const char *sym_name = "_puts";

  puts("[i] Dumping symbol table...");

  uint32_t index=0;
  uint32_t found=0;

  struct nlist_64 *entry;
  entry = (struct nlist_64*)__symtable;
  entry+=iundefsym;
  for (uint32_t i = 0; i < nundefsym; i++) {
    printf("entry {#%d}: %s\n", i, __strtable+entry->n_un.n_strx);
    if (strcmp(sym_name, __strtable+entry->n_un.n_strx) == 0) {
      printf("^^^ found! ^^^\n");
      index=i+iundefsym;
      found = 1;
    }
    entry = ((void*)entry + sizeof(struct nlist_64));
  }

  if (!found) {
    puts("dc non trovo il simbolo.\n");
    return;
  }

  printf("\n\tindex: ----> %d\n", index);

  uint32_t lazy_index=0;
  found=0;

  uint32_t *__indirect_symtable = base + indirectsymoff;
  for (uint32_t i = 0; i < nindirectsyms; i++) {
    printf("{%d} -> %d\n", i, __indirect_symtable[i]);
    if (__indirect_symtable[i] == index) {
      lazy_index=i;
      found=1;
      break;
    }
  }

  if (!found) {
    puts("dc non trovo il simbolo.\n");
    return;
  }

  printf("final index ----------> %d\n", lazy_index);

  uint64_t *__lazy = base + stuff;
  __lazy+=lazy_index;
  *__lazy=(uint64_t)kek;

}
