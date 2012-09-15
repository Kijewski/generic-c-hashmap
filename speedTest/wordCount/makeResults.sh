#!/bin/bash
set -e

echo "Hashing $(cat Clarissa.txt | wc -w) words"
echo

function measure() {
	NAME=$1
	shift
	$@ 2>&1 >/dev/null
	SUM=0
	for run in `seq 25`; do
		TIME="$((time -p ($@ 2>&1 >/dev/null)) 2>&1 | sed -ne 's/user //p')"
		#echo $TIME >&2
		SUM=$(dc -e "$SUM $TIME + p")
	done
	echo "$NAME: $(dc -e "3 k $SUM 25 / p")"
}

for contestant in generic-c-hashmap-count uthash-count; do
	for optimization in O0 O1 O2 O3 Os Ofast; do
		echo "Contestant: $contestant; optimization: -$optimization"
		measure "Compiling" cc -std=gnu99 "-$optimization" "./$contestant.c" -o "./$contestant" 
		measure "Execution" "./$contestant" Clarissa.txt
		echo
	done
done
