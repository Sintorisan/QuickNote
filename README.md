# Quick Note Plasma Widget

A small KDE Plasma 6 panel widget for file-based Markdown notes.

## Build

This project targets KDE Plasma 6 and uses a QML UI with a small C++/Qt backend.

On Arch/CachyOS, install:

```sh
sudo pacman -S \
  base-devel \
  cmake \
  extra-cmake-modules \
  qt6-base \
  qt6-declarative \
  qt6-tools \
  plasma-sdk \
  libplasma \
  kcoreaddons \
  kconfig \
  ki18n \
  kio \
  kpackage \
  ksvg \
  ninja
```
## Requirements


Run from the project root:

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build
```

After installing, add Quick Note to a Plasma panel.

If Plasma does not see it immediately, restart Plasma:

```sh
systemctl --user restart plasma-plasmashell.service
```

Or log out and back in.

## Shortcut

Quick Note exposes a Plasma action named `Toggle Quick Note` that opens or closes the existing panel popup. If your Plasma widget/global shortcut UI shows actions for this widget, assign a shortcut there. The action toggles the normal popup only; it does not open a separate window.
