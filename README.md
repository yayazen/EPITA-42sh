## EPITA-42sh

Code has been packaged with meson/ninja.

### Dependencies
* meson
* ninja-build
* libreadline.so.6
* libasan.so.6

On Ubuntu 20.04, run the following command to install dependencies:
```bash
sudo apt install meson libreadline-dev
```

### Building from source
```bash
meson setup build
# or with tests
meson setup -Dforce_check=true build

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

### Install/Uninstall
```bash
meson install
meson uninstall
```

### Resources
**42sh Grammar** --> [docs/42sh-grammar.md](doc/42sh-grammar.md)
