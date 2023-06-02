#!/bin/bash

if [[ $# < 2 ]]; then
	echo Too few arguments
	exit
elif [[ $# > 2 ]]; then
	echo Too many arguments
	exit
fi

filename=$1
numLines=$2


contents=`cat $filename`
read -r -a parties <<< "$contents"


random_name(){
	local name_size=`expr $RANDOM % 10 + 3`
	for (( i = 0; i < name_size; i++ )); do
		random_char=`expr $RANDOM % 24 + 65`
		random_char=`printf '%03o' $random_char`
		printf "%b" "\\$random_char"
	done
}


print_lines(){
	for (( i = 0; i < numLines; i++ )); do
		echo `random_name` `random_name` ${parties[`expr $RANDOM % ${#parties[@]}`]}	
	done
}

print_lines > inputFile
