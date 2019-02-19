#!/bin/bash

declare -a images;
declare -a threshold;
declare -a points_per_task;

RED='\e[31m'
YELLOW='\e[33m'
GREEN='\e[32m'
NC='\e[39m'

function init {
	
	images=(""
		"airplane"
		"beach"
		"camaro"
		"golden_gate"
		"hot_air_balloon"
		"merry_christmas"
		"ok"
		"santa"
		"snowman"
		"venice"
	);

	threshold=(0 10 20 30 40 50 60 70 80 90 110);	
	#task4_thresholds=(0 40 25 100 56 78 20 100 61 150 75);
	
	points_per_task=(0 1 2 3 3);

	numberOfTests=10

	total=0

	make build &>/dev/null
	if [ $? -ne 0 ]; then
		echo -e "${RED}Makefile build rule failed!\n"
		exit 1
	fi
}

function prepare_test {
	cp "inputs/${images[${1}]}.bmp" .
	cp "inputs/${images[${1}]}.bin" .
	echo ${images[${1}]}.bmp > input.txt
	echo ${threshold[${1}]} >> input.txt
	echo ${images[${1}]}.bin >> input.txt
}

function clean_test {
	image=${images[${1}]}
	
	#Clean input files.
	rm -rf ${image}.bmp
	rm -rf input.txt
	rm -rf ${image}.bin

	# Clean output files.
	rm -rf ${image}_black_white.bmp
	rm -rf ${image}_f1.bmp
	rm -rf ${image}_f2.bmp
	rm -rf ${image}_f3.bmp
	rm -rf "compressed.bin"
	rm -rf "decompressed.bmp"
	rm -rf *.bmp
}

function compare_files {
	# first argument is the output file
	# second argument is the ref file

	# No complicated testing, just compare.
	cmp ${1} ${2} &>/dev/null
	if [ $? -ne 0 ]; then
		return 0 # files differ
	else
		return 1 # files are identical
	fi
}

# First argument is the task rank - 1, 2, 3 or 4.
function test_failed {
	echo -e "\t Task ${1}    ${RED}FAILED${NC} -> 0 points"
}

function test_passed {
	echo -e "\t Task ${1}    ${GREEN}PASSED${NC} -> ${points_per_task[${1}]} point(s)"
	total=$((total+${points_per_task[${1}]}))
}

function check_test {
	echo -e "${YELLOW}Test ${1}${NC} ......................................"

	timeout 60 make -s run &>/dev/null
	if [ $? -ne 0 ]; then
		echo -e "${RED}Make run failed${NC} - no such target OR timeout OR program interrupted (e.g. SEGFAULT)."
		return
	fi

	image=${images[${1}]}

	# Task 1.
	compare_files "${image}_black_white.bmp" "ref/${image}_black_white.bmp"
	if [ $? -ne 1 ]; then
		test_failed 1 
	else
		test_passed 1
	fi

	# Task 2.
	compare_files "${image}_f1.bmp" "ref/${image}_f1.bmp"
	f1=$?
	compare_files "${image}_f2.bmp" "ref/${image}_f2.bmp"
	f2=$?
	compare_files "${image}_f3.bmp" "ref/${image}_f3.bmp"
	f3=$?
	if [ $f1 -eq 1 ] && [ $f2 -eq 1 ] && [ $f3 -eq 1 ]; then
		test_passed 2
	else 
		test_failed 2
	fi
 
	# Task 3.
	compare_files "compressed.bin" "ref/${image}_compressed.bin"
	if [ $? -ne 1 ]; then
		test_failed 3
	else
		test_passed 3
	fi
	
	# Task 4.
	compare_files "decompressed.bmp" "ref/${image}_decompressed.bmp"
	if [ $? -ne 1 ]; then
		test_failed 4
	else
		test_passed 4
	fi

	echo -en "\n"
}

# Firstly, init everything.
init

# Start testing.
for ((i=1; i<=$numberOfTests; i++))
do
	prepare_test $i
	check_test $i 
	clean_test $i
done

make clean &> /dev/null

# Output result.
echo -en "\n\t${YELLOW}TOTAL:${NC} $total out of 90\t"
if [ $total -eq 90 ]; then
	echo -e "${GREEN}Good job! ;)${NC}\n\n"
else 
	echo -e "\n\n"
fi
