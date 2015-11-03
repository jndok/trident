#include "trident.h"

__attribute__((weak_import)) extern int main(void); //-Wl,-U,_sym_name

void *__get_target_base_addr(void *sym) {
  struct dl_info info;
  if(dladdr(sym, &info) == 0)
    return NULL;

  return info.dli_fbase;
}

void hook(const char *target, void *replacement, struct mach_header *header_ptr)
{
  assert(target);
  assert(replacement);

  puts("[+] Injected into process, now attempting to retrieve binary image base address...");
  void *base=NULL;
/*
  if ((base=__get_target_base_addr((void*)main)) == NULL) {
    puts("[!] Failed to retrieve base address using _main symbol. The binary may be using another entry point or may be stripped.");
    exit(-1);
  } else {
    puts("[+] Correctly found a _main symbol in the address space. Be aware that this may not be the actual image entry point!");
  }
*/

  base=(void*)header_ptr;

  printf("[+] Found binary mach header @ %p\n", base);

  struct mach_header *header=(struct mach_header*)base;
  if (header->magic != MH_MAGIC_64) {
    puts("[!] Unsupported architecture. trident only works with x86_64 binaries at the moment.");
    exit(-2);
  } else {
    puts("[i] Found a valid architecture, proceeding with the hooking...");
  }

  hook_table_t table;
  struct load_command *loadcmd=base+sizeof(struct mach_header_64);
  for (uint32_t i = 0; i < header->ncmds; i++) {
    if (loadcmd->cmd==LC_SEGMENT_64) {
      struct segment_command_64 *segcmd=(struct segment_command_64*)loadcmd;
      if (strcmp(segcmd->segname, SEG_DATA) == 0) {
        struct section_64 *curr_sec=(struct section_64*)((void*)segcmd+sizeof(struct segment_command_64));
        for (uint32_t j = 0; j < segcmd->nsects; ++j) {
          if (strcmp(curr_sec->sectname, SECT_LAZY_PTRS) == 0) {
            table.i_indirectsyms=curr_sec->reserved1;
            table.lazy_offset=curr_sec->offset;
          }
          curr_sec=((void*)curr_sec+sizeof(struct section_64));
        }
      }
    } else if (loadcmd->cmd==LC_SYMTAB) {
      struct symtab_command *symtabcmd=(struct symtab_command*)loadcmd;
      table.__symtable = base + symtabcmd->symoff;
      table.__strtable = base + symtabcmd->stroff;
      table.__sym_off = symtabcmd->symoff;
      table.__str_off = symtabcmd->stroff;
      table.nsyms = symtabcmd->nsyms;
    } else if (loadcmd->cmd == LC_DYSYMTAB) {
      struct dysymtab_command *dysymtabcmd=(struct dysymtab_command*)loadcmd;
      table.i_undefsyms=dysymtabcmd->iundefsym;
      table.n_undefsyms=dysymtabcmd->nundefsym;
      table.__indirect_symtable = base + dysymtabcmd->indirectsymoff;
      table.n_indirectsyms=dysymtabcmd->nindirectsyms;
    }

    loadcmd=((void*)loadcmd+loadcmd->cmdsize);
  }

  void *undef_symtable=table.__symtable+(sizeof(struct nlist_64) * table.i_undefsyms);
  uint32_t *indirect_symtable=(uint32_t*)table.__indirect_symtable;
  struct nlist_64 *entry = (struct nlist_64*)undef_symtable;

  uint32_t symtable_index=0, lazy_index=0, found=0;

  for (uint32_t i = 0; i < table.n_undefsyms; ++i) {
    if (strcmp(target, (table.__strtable+entry->n_un.n_strx)) == 0) {
      symtable_index=table.i_undefsyms+i;
      found++;
      break;
    }

    entry=((void*)entry + sizeof(struct nlist_64));
  }

  if (!found) {
    printf("[!] Symbol \'%s\' was not found in the image's symbol table. Aborting.\n", target);
    exit(-3);
  } else {
    printf("[i] Symbol \'%s\' was correctly found in the symbol table. Calculating lazy pointer table index...\n", target);
  }

  found--;

  indirect_symtable+=table.i_indirectsyms;
  for (uint32_t i = 0; i < (table.n_indirectsyms - table.i_indirectsyms); ++i) {
    if (indirect_symtable[i] == symtable_index) {
      lazy_index=i;
      found++;
    }
  }

  if (!found) {
    printf("[!] Unable to calculate correct lazy index. Aborting.\n");
    exit(-4);
  } else {
    printf("[+] Calculated lazy index: %d! Overwriting lazy table entry...\n", lazy_index);
  }

  uint64_t *lazy_symtable=base+table.lazy_offset;
  lazy_symtable+=lazy_index;
  printf("[i] Overwriting lazy table entry %#llx @ index %d with  replacement -> %#llx\n", *lazy_symtable, lazy_index, (uint64_t)replacement);

  *lazy_symtable=(uint64_t)replacement; //hook

}
