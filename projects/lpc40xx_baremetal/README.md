This project demonstrates absolute minimum code that is needed to bring-up the Cortex-M4 CPU.

Build this project with: `scons --project=lpc40xx_baremetal --no-float-format`

If you get an error, such as `undefined reference to '_sbrk'`, then you either use `--no-float-format` or you may be using a C library function that relies on `malloc()`. In this case, you need to reference `sbrk.c` implementation from the `lpc40xx_freertos` project.

## Newlib

> Newlib is a C standard library implementation intended for use on embedded systems. It is a conglomeration of several library parts, all under free software licenses that make them easily usable on embedded products.

There are a couple of settings at `env_arm` file of relevance:

```
    LINKFLAGS=[
        "-nostartfiles",
        "-specs=nano.specs",
        "-specs=nosys.specs",
    ],
```

### `-nostartfiles`

We don't have a loader to initialize our C program (e.g. loader that comes prepackaged with an OS) so we need to write the logic to initialize RAM ourselves.

GNU documentation states that:

> Do not use the standard system startup files when linking. The standard system libraries are used normally, unless -nostdlib or -nodefaultlibs is used.

See this [Stack Overlow post](https://stackoverflow.com/a/29429820/12254460)

### `-specs=nano.specs`

This provides smaller footprint for C library functions like `printf()` or `malloc()`. Not linking with this option will result is larger code generated for your compiled image.

For this sample project, when you use a standard library function like `puts()`, there will be notable differences in the compiled size. The difference may be even larger when using `malloc()` function.

Using this option results in:
```
   text    data     bss     dec     hex filename
   7252    2112      96    9460    24f4 _build_lpc40xx_baremetal/lpc40xx_baremetal.elf
```

Not using this option results in:
```
   text    data     bss     dec     hex filename
   3024     100      28    3152     c50 _build_lpc40xx_baremetal/lpc40xx_baremetal.elf
```

### `-specs=nosys.specs`

> Nosys library contains set of default syscall stubs, majority of the stubs just returns failure.

This link option indicates that there is no "system", and hence the GCC will provide minimal, and default implementation for things like the `_write()` or `_sbrk()` functions.