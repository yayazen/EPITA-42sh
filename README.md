## EPITA-42sh

Code has been packaged with autotools.

## Install dependencies (Debian, Ubuntu)
```bash
sudo apt install -y autogen autoconf automake libtool python
```

### Building from source
```bash
# Build the application
./autogen.sh
# or
autoreconf --install
mkdir build
cd build
../configure && make

# Build with DEBUG flag
make clean && make CFLAGS="-DDEBUG"

# Run tests
make check
```

### Installation
```bash
sudo make install
# or
sudo install -m 0755 -o root ./42sh /usr/local/bin/
```

### Usages
```bash
42sh [] []
```

### Resources
**42sh Grammar**  -->  [docs/42sh-grammar.md](doc/42sh-grammar.md)


### Fix precommit
If you get this error when attempting to commit:
```bash
.git/hooks/pre-commit: 2: exec: Scripts/git-pre-commit-hook: not found
```

Run these commands:
```bash
pre-commit install
rm .git/hooks/pre-commit.legacy
```

And try to commit again