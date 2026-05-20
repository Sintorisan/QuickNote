# Quick Note

A small KDE Plasma 6 widget for file-based Markdown notes.

Notes are stored as normal Markdown files in a folder you choose:

```text
current.md
archive/
```

## Install

Install the Plasma 6, Qt 6, and KDE Frameworks 6 development packages for your distribution, plus CMake and a C++ compiler.

Then run:

```sh
git clone https://github.com/Sintorisan/QuickNote.git
cd QuickNote
chmod +x install.sh
./install.sh
```

Restart Plasma Shell or log out and back in, then add **Quick Note** to your panel from the widget picker.

## Manual Install

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build
```

## Uninstall

```sh
kpackagetool6 --type Plasma/Applet --remove org.sintori.quicknote
rm -rf ~/.local/share/plasma/plasmoids/org.sintori.quicknote
```