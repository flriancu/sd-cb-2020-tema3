#!/bin/bash

#
# Tema3 Test Suite
#
# 2020, SD
#

# ----------------- General declarations and util functions ------------------ #

INPUT_DIR=_test/input/
REF_DIR=_test/ref/
OUT_DIR=_test/output/
COMMANDS_DIR=_test/commands/
EXEC_NAME=./tema3
VALGRIND="valgrind --leak-check=full --error-exitcode=1"
max_points=85
max_bonus=20
result_points=0
bonus_points=0

print_header()
{
	header="${1}"
	header_len=${#header}
	printf "\n"
	if [ $header_len -lt 71 ]; then
		padding=$(((71 - $header_len) / 2))
		for ((i = 0; i < $padding; i++)); do
			printf " "
		done
	fi
	printf "= %s =\n\n" "${header}"
}

test_do_fail()
{
	printf "failed  [ 0/%02d]\n" "$max_points"
	if test "x$EXIT_IF_FAIL" = "x1"; then
		exit 1
	fi
}

test_do_pass()
{
	printf "passed  [%02d/%02d]\n" "${1}" "$max_points"
	((result_points+=${1}))
}

bonus_function()
{
	printf "passed\n"
	((bonus_points+=${1}))
}

test_function()
{
	input_file="$INPUT_DIR${1}"
    commands_file="$COMMANDS_DIR${2}"
	output_file="$OUT_DIR${3}"
	ref_file="$REF_DIR${4}"
	points_per_test="${5}"
	points_per_test_vg="${6}"
    
	$EXEC_NAME $input_file $commands_file $output_file 

	diff $output_file $ref_file > /dev/null
	if test $? -eq 0; then
		printf "[%s]..........................................." ${2}
		test_do_pass $points_per_test
		#valgrind test

		$VALGRIND $EXEC_NAME $input_file $output_file &> /dev/null
		if [ $? -eq 0 ]; then
			printf "[VALGRIND: PASSED]...................................." ${2}
			bonus_function $points_per_test_vg
		else
			printf "[VALGRIND: FAILED]...................................." ${2}
		fi

	else
		printf "[%s]..........................................." ${2}
		test_do_fail
	fi
}

init_function()
{
    make build
	mkdir $OUT_DIR &> /dev/null
}

clean_out_function()
{
    rm _test/output/*.*
    rmdir _test/output
}

init_function

print_header "Tema 3 SD HTML Parser"

#Testing

declare tests_values_1=(1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2) # 29
declare tests_values_2=(2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3 3 3 3 3 3 3) # 56
declare valgrind_tests_values=1

for i in {1..22}
do
	test_function "colorbars-small.html" "commands-colorbars-$i.in" "colorbars-small-$i.html" "colorbars-small-$i.html" "${tests_values_1[$i-1]}" "${valgrind_tests_values}" 
	echo ""
done

for i in {1..22}
do
	test_function "colorbars-big.html" "commands-colorbars-$i.in" "colorbars-big-$i.html" "colorbars-big-$i.html" "${tests_values_2[$i-1]}" "${valgrind_tests_values}" 
	echo ""
done

#end Testing

printf "\n%49s [%02d/$max_points]\n" "Total:" $result_points;
if [ $bonus_points -ne 44 ]; then
	((bonus_points=0))
	echo "Failed VALGRIND tests"
else
	((bonus_points=$max_bonus))
fi
printf "\n%49s [%02d/$max_bonus]\n" "Bonus:" $bonus_points;

clean_out_function

echo 

