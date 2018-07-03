#!/bin/bash
set -e
pushd tool
make
popd
pushd FS_Dumper
make
popd
mkdir -p bin
rm -f bin/FS_Dumper.bin
cp FS_Dumper/FS_Dumper.bin bin/FS_Dumper.bin
mkdir -p html_payload
tool/bin2js bin/FS_Dumper.bin > html_payload/payload.js
FILESIZE=$(stat -c%s "bin/FS_Dumper.bin")
PNAME=$"FS Dumper"
cp exploit.template html_payload/FS_Dumper.html
sed -i -f - html_payload/FS_Dumper.html << EOF
s/#NAME#/$PNAME/g
s/#BUF#/$FILESIZE/g
s/#PAY#/$(cat html_payload/payload.js)/g
EOF
rm -f FS_Dumper/FS_Dumper.bin
rm -f html_payload/payload.js
