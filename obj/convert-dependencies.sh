#!/bin/sh
# AUTO-GENERATED FILE, DO NOT EDIT!
if [ -f $1.org ]; then
  sed -e 's!^D:/cygwin/lib!/usr/lib!ig;s! D:/cygwin/lib! /usr/lib!ig;s!^D:/cygwin/bin!/usr/bin!ig;s! D:/cygwin/bin! /usr/bin!ig;s!^D:/cygwin/!/!ig;s! D:/cygwin/! /!ig;s!^X:!/cygdrive/x!ig;s! X:! /cygdrive/x!ig;s!^W:!/cygdrive/w!ig;s! W:! /cygdrive/w!ig;s!^D:!/cygdrive/d!ig;s! D:! /cygdrive/d!ig;s!^C:!/cygdrive/c!ig;s! C:! /cygdrive/c!ig;' $1.org > $1 && rm -f $1.org
fi
