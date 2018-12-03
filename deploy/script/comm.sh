#!/bin/sh

monitor()
{
	num=`ps aux | grep -w $1 | grep -w \`whoami\` | grep -v grep | wc -l`;
	if [ $num -eq 0 ]; then
		echo "[Stop] $1 ... attention!"
	else
		echo "[Running] $1 ... OK!"
	fi
}

