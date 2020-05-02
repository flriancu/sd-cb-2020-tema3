import os
import sys
import argparse


class RET:
    OK = 0
    FAIL = 1


# Printing options
def eprint(*s):
    args = s
    print("[error] ".rjust(10), *args)


def iprint(*s):
    args = s
    print("[info] ".rjust(10), *args)


def make_parser():
    parser=argparse.ArgumentParser()

    parser.add_argument("-i", "--input", required=True, help="Master commands file")
    parser.add_argument("-o", "--output", required=True, help="Path to directory where output files will be stores")

    return parser


def main():
    ret = RET.OK
    
    # Parse arguments
    parser = make_parser()
    args = parser.parse_args()
    iprint(args)

    if ret == RET.OK:
        if not os.path.isfile(args.input):
            eprint("Master commands file does not exist")
            ret = RET.FAIL

    # Read the master commands file
    if ret == RET.OK:
        try:
            with open(args.input, 'r') as fi:
                lines = fi.readlines()
        except Exception as e:
            eprint(str(e))
            ret = RET.FAIL

    # Generate as many test files as there are commands in the master file
    if ret == RET.OK:
        lines = [l for l in lines if l.strip()]
        nb = int(lines[0])
        if nb >= len(lines):
            eprint("Not enough lines in the master file")
            ret = RET.FAIL
        
        lines = [l.strip() for l in lines]
        if "format" in lines:
            eprint("The master file must not contain the format command!")
            ret = RET.FAIL

    if ret == RET.OK:
        iprint("Will generate %d command files" % (nb))
        name = os.path.splitext(os.path.basename(args.input))[0]
        try:
            for i in range(1, nb + 1):
                with open(args.output + os.sep + name + "-" + str(i) + ".in", 'w') as fo:
                    fo.write("%d\n" % (i + 1))
                    for j in range(1, i + 1):
                        fo.write(lines[j] + "\n")

                    # Master file does not have format, so we add it last
                    fo.write("format\n")
        except Exception as e:
            eprint(str(e))
            ret = RET.FAIL

    if ret == RET.OK:
        iprint("Command files generated at:", args.output)

    return ret


if __name__ == "__main__":
    ret = main()
    sys.exit(ret)
