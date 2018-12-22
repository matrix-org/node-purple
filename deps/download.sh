#!/bin/bash

PURPLE_VER="2.13.0"
PURPLE_DIR="./pidgin-$PURPLE_VER"
PURPLE_TAR="./purple.tar.bz2"

LIB_TAR="./libpurple.tar.xz"
LIB_DIR="./libpurple"

# Change to the current directory
cd "$(dirname "$0")"

ARCH=`uname -m`

# Check the arch
if [ "$ARCH" == "x86_64" ]
then
    echo "This IS a supported architecture"
else
   echo "Your architecture is not currently supported"
   exit 1;
fi

# Donwload the headers
if [ -d $PURPLE_DIR ]; then
  echo "purple already exists, not doing anything"
  exit 0;
else
  echo "Downloading purple headers"
  wget "https://vorboss.dl.sourceforge.net/project/pidgin/Pidgin/$PURPLE_VER/pidgin-$PURPLE_VER.tar.bz2" -O purple.tar.bz2
fi

# Download the libaries
if [ -d $LIB_DIR ]; then
  echo "libpurple already exists, not doing anything"
  exit 0;
else
  echo "Downloading purple libraries"
  wget "https://half-shot.uk/files/libpurple/libpurple-$PURPLE_VER-$ARCH.tar.xz" -O libpurple.tar.xz
fi

echo "Checking checksums, expected values below:"
cat checksum.sha256

sha256sum -c checksum.sha256

if [ $? != 0 ]; then
  echo "Checksums failed, not continuing"
  rm purple.tar.bz2
  rm libpurple.tar.xz
  exit 1;
fi

echo "Extracting files"
tar -xf purple.tar.bz2
mkdir $LIB_DIR
tar -xf libpurple.tar.xz -C $LIB_DIR

echo "Removing archives"
rm purple.tar.bz2
rm libpurple.tar.xz

echo "Finished downloading"
