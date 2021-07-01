#!/bin/bash
# 荣涛 2021年6月30日
# 经过讨论，决定decoder没有复杂的功能(排序，查找等)
# 这个脚本意在解决 decoder 已支持查找/命令行等功能

DECODER_PATH=/usr/bin/
DECODER=fdecoder

META=fastlog.metadata
LOGS=fastlog.log
OUTPUT=fastlog.txt

function show_help()
{
	echo ""
	echo "$0 -m [metafile] -l [logfile1,file2,file3...] -o [output.txt]"
	echo ""
	echo "	-m: 元数据文件名，默认为‘fastlog.metadata’"
	echo "	-l: 日志数据文件名，可输入多个文件，用','分隔，默认为‘fastlog.log’"
	echo "	-o: 输出文本文件名，默认为‘fastlog.txt’"
	echo ""
	echo " for example: $0 -m meta.bin -l file1.bin,file2.bin,file3.bin -o output.txt"
	echo ""
	echo " 如果想使用更详细的信息，请 $DECODER_PATH$DECODER -h 查看使用方法"
	echo ""
	exit 1
}



function parse_args()
{
    argvs=($(echo "$@"))
    elements=$[ $# - 1 ]
    for (( i = 0; i <= $elements; i++ ))
    {
        # 帮助信息
        if [ ${argvs[$i]} = "-h" ]; then
            show_help
            exit 1
        fi
        # 元数据
        if [ ${argvs[$i]} = "-m" ]; then
            META=${argvs[${i}+1]}
        fi
        # 日志数据
        if [ ${argvs[$i]} = "-l" ]; then
            LOGS=${argvs[${i}+1]}
        fi
        # 输出
        if [ ${argvs[$i]} = "-o" ]; then
            OUTPUT=${argvs[${i}+1]}
        fi
    }
}

if [ ! -e $DECODER_PATH/$DECODER ]; then
	DECODER_PATH=./
fi

if [ ! -e $DECODER_PATH/$DECODER ]; then
	echo "ERROR: fdecoder not install."
	exit 1
fi

if [ $# -eq 0 ]; then
	if [ ! -e $META ]; then
		echo "$META not exist."
		show_help
	fi
	if [ ! -e $LOGS ]; then
		echo "$LOGS not exist."
		show_help
	fi
else
	parse_args $*
fi


echo "Metadate File is $META"
echo "Logdate File is $LOGS"
echo "Output File is $OUTPUT"



# -q: 安静的解析
# --cli off: 关闭cli
$DECODER_PATH$DECODER -q --cli off -M $META -L $LOGS -o $OUTPUT

