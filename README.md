## EPITA-42sh

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
meson setup -Dforce_check=true build

meson compile -C build
```

### Build documentation
```bash
meson setup -Ddoc=true build
meson compile -C build doxygen_doc

xdg_open build/doxygen_doc/index.html
```

### Install/Uninstall
```bash
meson install
meson uninstall
```

### Resources
**42sh Grammar** --> [docs/42sh-grammar.md](doc/42sh-grammar.md)
