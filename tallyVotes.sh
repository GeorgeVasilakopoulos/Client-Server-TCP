#!/bin/bash


if [[ $# < 1 ]]; then
	echo "Too few arguments"
	exit
elif [[ $# >1 ]]; then
	echo "Too many arguments"
	exit
fi

filename=$1


if [[ ! -e inputFile ]]; then
	echo inputFile does not exist
	exit
fi

IFS=$'\n' readarray -t parties <<< `awk '!voted[$1]++' inputFile | cut -f2 --delim=' ' | sort | uniq -c`


echo_output(){
	counter=0
	for elem in ${parties[@]}; do
		if (( $counter % 2 == 0 )); then
			votesNumber=$elem
		else
			echo $elem $votesNumber
		fi
		((counter+=1))
	done
}

echo_output > $filename
