#!/bin/sh

cd "$(dirname "$0")"
TOP="$PWD"
set -e

if [ -e /usr/bin/python2 ]; then
  export PYTHON=/usr/bin/python2
fi

if ! [ -e "libtorrent-src" ]; then
  svn co svn://svn.code.sf.net/p/libtorrent/code/tags/libtorrent-1_0_1/ libtorrent-src
fi

cd libtorrent-src
svn up
./autotool.sh
./configure --prefix="$TOP/install"
make
make install

