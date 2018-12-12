#!/bin/bash

MPOINT="./mount-point"
FILE1="fuseLib.c"
FILE2="fuseLib.h"
FILE3="MyFileSystem.c"

cp ./src/$FILE1 $MPOINT/
cp ./src/$FILE2 $MPOINT/

[ -d ./temp ] && rm -r ./temp
mkdir ./temp

cp ./src/$FILE1 ./temp/
cp ./src/$FILE2 ./temp/
cp ./src/$FILE1 $MPOINT/
cp ./src/$FILE2 $MPOINT/

./my-fsck virtual-disk

diff ./temp/$FILE1 $MPOINT/$FILE1
diff ./temp/$FILE2 $MPOINT/$FILE2

truncate -o --size=-1 ./temp/$FILE1
truncate -o --size=-1 $MPOINT/$FILE1

./my-fsck virtual-disk

diff ./src/$FILE1 $MPOINT/$FILE1
diff ./src/$FILE1 ./temp/$FILE1

cp ./src/$FILE3 $MPOINT/
cp ./src/$FILE3 ./temp/

diff ./temp/$FILE3 $MPOINT/$FILE

truncate -o --size=+2 ./temp/$FILE2
truncate -o --size=+2 $MPOINT/$FILE2

./my-fsck virtual-disk

diff ./src/$FILE2 $MPOINT/$FILE2
diff ./src/$FILE2 ./temp/$FILE2
