#!/bin/bash
set -e

ARCHIVE="xmervaj00.zip"
TESTDIR="is_it_ok_tmp"

# always start clean
rm -rf "$TESTDIR"
mkdir "$TESTDIR"

# pack files, flatten the directory
zip -j "$ARCHIVE" ./include ./src ./Makefile ./rozdeleni

# run the check
./is_it_ok.sh "$ARCHIVE" "$TESTDIR"
