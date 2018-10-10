#! /bin/bash

export FIRSTPATH=`pwd`
ERROR=1
print_error () {
	echo "$1"
	ERROR=0
}
end () {
	cd $FIRSTPATH
	exit $ERROR
}


[ -x mytar ] || print_error "Non existing executable"
[ $ERROR -eq 1 ] || end
[ -d tmp ] && rm -r tmp
mkdir tmp
cd tmp
echo "Hello world!" > file1.txt
head -10 /etc/passwd > file2.txt
head -c 1024 /dev/urandom > file3.dat
../mytar -c -f filetar.mtar file1.txt file2.txt file3.dat
mkdir out
cp ./filetar.mtar ./out/filetar.mtar
cd ./out
../../mytar -x -f filetar.mtar

if [ -e file1.txt ] ; then
	diff ./file1.txt ../file1.txt || print_error "files file1.txt are different"
else
	print_error "file1.txt was not created"
fi

if [ -e file2.txt ] ; then
	diff ./file2.txt ../file2.txt || print_error "files file2.txt are different"
else
	print_error "file2.txt was not created"
fi

if [ -e file3.dat ] ; then
	diff ./file3.dat ../file3.dat || print_error "files file3.dat are different"
else
	print_error "file3.dat was not created"
fi

[ $ERROR -eq 0 ] || echo "Correct"
end
