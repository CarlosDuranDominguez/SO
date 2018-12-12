#!/.bin/bash
[-d ./tmp] rm -r ./tmp
mkdir ./tmp
cp ./src/myFS.h ./tmp/myFS.h
cp ./src/myFS.h ./mount-point/myFS.h
./my-fsck virtual-disk
diff ./tmp/myFS.h ./mount-point/myFS.h
mv ./mount-point/myFS.h ./mount-point/myFS2.h
./my-fsck virtual-disk
diff ./tmp/myFS.h ./mount-point/myFS2.h
cp ./src/MyFileSystem.c ./tmp/.hidden.c
cd ./mount-point
ls
cd ../
./my-fsck virtual-disk

