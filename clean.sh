#!/bin/bash
pushd tool
make clean
popd
pushd FS_Dumper
make clean
popd
rm -f html_payload/FS_Dumper.html
rm -f bin/FS_Dumper.bin

