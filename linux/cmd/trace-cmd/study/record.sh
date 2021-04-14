#!/bin/bash
# 荣涛

function _report() {
	trace-cmd report --profile | more
	rm -f trace.dat
}

function _sched_switch() {
	trace-cmd record -e sched_switch ls >> /dev/null
	_report
}

function _function_graph() {
	#-p run command with plugin enabled
	sudo trace-cmd record -p function_graph -l vfs_* -F
	_report
}

function _g() {
	# 抓取函数内部所有子函数的执行时间，需要使用-g func选项
	sudo trace-cmd record -p function_graph -g vfs_read -F
	_report
}

_sched_switch
_function_graph
