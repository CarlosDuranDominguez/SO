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

if [ -x mytar ] ; then
	if [ -d tmp ] ; then 
		rm -r tmp
	fi

	mkdir tmp
	cd ./tmp
	echo "Hello world!" > file1.txt
	head -10 /etc/passwd > file2.txt
	head -c 1024 /dev/urandom > file3.dat
	dd if=/dev/zero count=8 of=file4.dat
	
	../mytar -cf mytarfile file1.txt file2.txt file3.dat file4.dat
	../mytar -rf mytarfile file1.txt
	../mytar -if mytarfile
	mkdir out
	cp ./mytarfile ./out/mytarfile	
	cd ./out
	../../mytar -xf mytarfile

	if [ -e file1.txt ] ; then
		print_error "there is file1.txt"
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

	if [ -e file4.dat ] ; then
		diff ./file4.dat ../file4.dat || print_error "files file3.dat are different"
	else
		print_error "file3.dat was not created"
	fi
		
else
	echo "there is not mytar executable"
fi

