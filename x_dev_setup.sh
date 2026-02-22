#!/bin/sh

# Prepare Linux environment for Mailager development (dev libs & IDE)

apt install libcurl4-openssl-dev
apt install libmimetic-dev
apt install libsecret-1-dev
apt install libwxgtk-webview3.2-dev

apt install codeblocks
# Note: likely makes sense to install Code::Blocks manually using a newer package
# , see https://www.codeblocks.org/downloads/binaries/#imagesoslinux48pnglogo-linux-32-and-64-bit

