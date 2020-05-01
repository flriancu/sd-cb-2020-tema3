How to run the homework (from the current directory):

../src/tema3 colorbars-small.html commands-colorbars.in _result.html
../src/tema3 colorbars-big.html commands-colorbars.in _result.html

How to generate input HTML files (from the current directory):

python3 gen_page.py -i colorbars.jpg -m image -w 4 -o _test/input/colorbars-small.html
python3 gen_page.py -i colorbars.jpg -m image -w 20 -o _test/input/colorbars-big.html

How to generate the commands:

python3 gen_commands.py -i commands-colorbars.in -o _test/commands

How to generate the reference outputs:

./gen_results.sh
