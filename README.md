## 42sh

Code has been packaged with meson/ninja.

### Dependencies
* meson
* ninja-build
* libreadline.so.6
* libasan.so.6


### Building from source
```bash
meson setup build
# or with tests
meson setup -Dtestunit=true -Dforce_check=true build

meson compile -C build

# On Ubuntu 20.04, this command will not work if
# using meson from packages. Use this command instead:
ninja -C builddir
```

### Build documentation
```bash
meson setup -Ddoc=true build
meson compile -C build doxygen_doc

xdg_open build/doxygen_doc/index.html
```


### Enable debugging symbols
```bash
meson setup --debug build --reconfigure
```

### Install/Uninstall
```bash
meson install
meson uninstall
```

### Run functional test
It is possible to run functional tests against both `dash` and `42sh`:
```bash
# Test on dash
tests/run_tests dash

# Test on 42sh
tests/run_tests build/42sh
```

### Coding style check
It is possible to check compliance with Epita's coding style using the following command (Ubuntu / Debian only). On the first run, a binary will be downloaded from Pierre Hubert's server:
```bash
tests/check_coding_style
```

### Resources
**42sh Grammar** --> [docs/42sh-grammar.md](doc/42sh-grammar.md)