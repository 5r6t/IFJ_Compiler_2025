#!/bin/bash
set -e

ARCHIVE="xmervaj00.zip"
TESTDIR="is_it_ok_tmp"

# always start clean
rm -rf "$TESTDIR"
mkdir "$TESTDIR"

# pack files, flatten the directory
zip -j -r "$ARCHIVE" ./include ./src ./Makefile ./rozdeleni dokumentace.pdf

# run the check
./is_it_ok.sh "$ARCHIVE" "$TESTDIR"