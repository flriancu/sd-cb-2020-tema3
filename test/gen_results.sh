
INPUT_DIR=_test/input/
OUT_DIR=_test/ref/
COMMANDS_DIR=_test/commands/
EXEC_NAME=../src/tema3


test_function()
{
	input_file="$INPUT_DIR${1}"
    commands_file="$COMMANDS_DIR${2}"
	output_file="$OUT_DIR${3}"
    
	$EXEC_NAME $input_file $commands_file $output_file
}


for i in {1..8}
do
	test_function "simple.html" "commands-simple-$i.in" "simple-$i.html"
done

for i in {1..24}
do
	test_function "colorbars-big.html" "commands-colorbars-$i.in" "colorbars-big-$i.html"
done

for i in {1..24}
do
	test_function "colorbars-small.html" "commands-colorbars-$i.in" "colorbars-small-$i.html"
done
