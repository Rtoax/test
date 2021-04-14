#!/bin/sh
# 荣涛 

function cleanall {
	filename=("*.o" "*.d" "*.gch" "*~"  "*.out" "core.*" "vgcore.*" "*.exe" "*.so" "*.a" "perf.data*")
	for name in ${filename[@]}; do
	    #echo $name
	    find . -name $name -type f -print -exec rm -rf {} \;
	done 
	return 0
}

function clean {
	echo $PWD | grep "\/test" 2>&1 > /dev/null
	if [ $? != 0 ]; then
		return 1
	fi
	cleanall
	return 0
}

function cleandir {
	if [ $# -lt 1 ]; then
		echo "Usage: $0 [directory list]"
		return 0
	fi
	QA2 "Are you sure to remove \"$*\" ? "
	if [ $? != 0 ]; then
		return 0
	fi
	for dir in $*
	do
		if [ -d $dir ]; then
			cd $dir
			# 删除一般文件
			rm -rf *
			# 删除隐藏文件
			rm -rf `find -name "\.[0-9a-zA-Z]*"`
			echo Clean directory $dir for further. `date` > CLEANDIR
			cd ..
		fi
	done
	return 0
}
