# trident
## info
_trident_ is a dynamic mach-o hooking library. It can override pointers in the lazy section in mach-o binaries, thus redirecting calls to other functions.

## usage
copy the `trident/` folder in your project dir and `#include "trident.h"`.

call the `hook` routine to set hooks. note that this should be done from injected libraries to hook functions in a specific image.

```
void hook(const char *target, void *replacement, struct mach_header *header_ptr)
```
- `target`: The target function symbol in the address space.
- `replacement`: The replacement function, to which redirect the call.
- `header_ptr`: The ASLR-slided runtime `mach_header` pointer of the image which the lazy section needs to be modified.

## mach_header
you are responsible for providing _trident_ the correct `mach_header` pointer. This is like saying which image in the address space needs to be edited to place the hooks.

There are various ways to achieve this. _trident_ example uses the `ProgramVars` structure, another way may be to use the `dladdr` function to dynamically get the image base address from the injected _dylib_. You need to know at least one local symbol exported by the image, and `weak_import` that into your _dylib_, then pass it to `dladdr`.<br>A common symbol to use could be the `main` function from a binary.

## injection
To inject your _dylibs_ just use the good old __dyld__-provided environment variables, or check this cool `task_for_pid` injection from my man [qwertyoruiop](https://twitter.com/qwertyoruiop) (supporting KJC): [inj](https://github.com/kpwn/inj).
Please note that the fucking SIP on El Capitan **will** be standing in your way when trying to hook protected binaries, and also when using `task_for_pid` Mach trap.

## notes
_trident_ sucks. Literally, it is no real good for now. it may be improved in the future, but for now it is a mess. it has many problems with objc binaries, and the dynamic localization of the `mach_header` makes it a bit sloppy, since additional work may need to be performed on different binaries to correctly hook.

if you need something more serious and stable, check out my other project [_harpoon_](https://github.com/jndok/harpoon).<br>_harpoon_ is very different from _trident_. it modifies directly the real function, so the redirection is performed inside of it. this makes it work on basically any binary, without problems.
