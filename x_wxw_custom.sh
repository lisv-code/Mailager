#!/bin/sh

# wxWidgets custom build.
# Run this script in the directory where wxWidgets sources are unpacked

# GTK 3 is used by default
# so at least libgtk-3-dev package is required (for Debian-based)
#  additionally
#   libwebkit2gtk-4.0-dev is required to use wxWebView
#   libsecret-1-0 is required to use wxSecretStore
# ... see https://docs.wxwidgets.org/stable/plat_gtk_install.html

mkdir gtk-build
cd gtk-build
../configure --enable-unicode --disable-shared --with-gtk

make

sudo make install

wx-config --version

