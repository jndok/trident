# trident
Here is a short introduction to _trident_.
## what is trident?
_trident_ is a Mach-O hooking engine, which works by overwriting entries in the lazy pointers table, a section found in every Mach-O binary which has unresolved symbols to lazy-bind.
<br>
_trident_ is obviously not complete, what you can see here is only the bare-bones code, it still lacks x86 support, proper error-checking, appropriate address space image searching, and general reliability.

I'll update the code in the next days/weeks/months and try to fix/improve it.

## how does it work?
Simply put: every binary that needs runtime linking has a table defining all the external symbols that need to be linked by __dyld__. _trident_ works by overwriting entries (pointers) in the lazy pointers table.
<br>
This way our overridden address will be used when calling the function, instead of the original lazily linked one.

## how do I use it?
```
cd trident-master
make
scripts/run.sh test
```
Call the `hook` function to overwrite entries:
```
hook(const char *sym_name, void *replacement) //attach a leading underscore to sym_name!
```

## injection
To inject your _dylibs_ just use the good old __dyld__-provided environment variables, or check this cool `task_for_pid` injection from my man [qwertyoruiop](https://twitter.com/qwertyoruiop) (supporting KJC): [inj](https://github.com/kpwn/inj).
Please note that the fucking SIP on El Capitan may be standing in your way when trying to hook protected binaries, and also when using `task_for_pid` Mach trap.
