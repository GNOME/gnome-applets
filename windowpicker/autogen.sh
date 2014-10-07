#! /bin/sh
autoreconf --verbose --install --symlink || exit 1
./configure "$@"
