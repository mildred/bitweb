#!/bin/sh

cd "$(dirname "$0")"
git clean -idx -e libtorrent-src -e install -e CMakeLists.txt.user
