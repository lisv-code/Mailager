#!/bin/sh
set -e

BUILD_VERSION=${1:?Error: BUILD_VERSION value is required}
BUILD_TARGET=${2:-"Release"}

echo Build target: $BUILD_TARGET, version: $BUILD_VERSION.

# Copy artifacts
SRC_BIN_DIR=../../DesktopApp/bin/$BUILD_TARGET
cp "$SRC_BIN_DIR/Mailager" ./mailager/usr/bin/
chmod 755 ./mailager/usr/bin/Mailager
cp "$SRC_BIN_DIR/Mailager.cfg" ./mailager/usr/etc/
chmod 644 ./mailager/usr/etc/Mailager.cfg
# SRC_RES_DIR=../../Resource
# cp "$SRC_RES_DIR/AppMain.png" ./mailager/usr/share/icons/mailager.png

# Backup template files
cp ./mailager/DEBIAN/control ./control.bkp

# Update template values
sed -i "s|#PKG_VERSION#|$BUILD_VERSION|g" ./mailager/DEBIAN/control

# Build package
dpkg-deb --build mailager
PKG_FILE_NAME=mailager_${BUILD_VERSION}_amd64.deb
mv mailager.deb $PKG_FILE_NAME
echo Result package: $PKG_FILE_NAME

# Restore template files
mv ./control.bkp ./mailager/DEBIAN/control

