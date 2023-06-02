#!/bin/bash


if [[ $# < 1 ]]; then
	echo "Too few arguments"
	exit
elif [[ $# >1 ]]; then
	echo "Too many arguments"
	exit
fi

pollLogFile=$1


if [[ ! -e $pollLogFile ]]; then
	echo $pollLogFile does not exist
	exit
fi

if [ ! -r $filename ]; then
	echo Permission to read $filename denied
	exit
fi

IFS=$'\n' readarray -t parties <<< `awk '!voted[$1 $2]++' $pollLogFile | cut -f3 --delim=' ' | sort | uniq -c`


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

# sort primarily according to votes count and secondarily in decreasing alphabetical order
echo_output | sort -nr -k2,2 -k1 > pollerResultsFile
