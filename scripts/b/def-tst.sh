#! /bin/sh

if false; then
 echo "Not reached."
elif [ -f ./configure ]; then
        c=./configure
elif [ -f ../configure ]; then
        c=../configure
else
  echo "Not in right place, dying."
  exit 1;
fi

$c --enable-debug --enable-tst-noassert-loop --with-fmt-float=glibc $@ && make clean && make check