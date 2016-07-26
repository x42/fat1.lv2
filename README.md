fat1.lv2 - AutoTuner-1
======================

fat1.lv2 is an auto-tuner based on Fons' zita-at1.

Install
-------

Compiling fat1.lv2 requires the LV2 SDK, gnu-make, and a c-compiler.

```bash
  git clone git://github.com/x42/fat1.lv2.git
  cd fat1.lv2
  make
  sudo make install PREFIX=/usr
```

Note to packagers: The Makefile honors `PREFIX` and `DESTDIR` variables as well
as `CFLAGS`, `LDFLAGS` and `OPTIMIZATIONS` (additions to `CFLAGS`), also
see the first 10 lines of the Makefile.
