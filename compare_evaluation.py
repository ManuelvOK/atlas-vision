#! /usr/bin/env python
import argparse
import sys

from typing import IO


def process_cmd_args():
    aparser = argparse.ArgumentParser()
    aparser.add_argument('atlas_input', metavar='ATLAS_EVAL', type=str,
                         help='atlas evaluation file')
    aparser.add_argument('cbs_input', metavar='CBS_EVAL', type=str,
                         help='cbs evaluation file')
    aparser.add_argument('-o', '--output', help='output file')
    return aparser.parse_args()


def parse_eval(input_file: str):
    tard = []
    with open(input_file) as f:
        for line in f:
            if not len(line):
                continue
            if line[0] == 't':
                tard.append(int(line.split(' ')[2]))
    return tard


def compare(atlas_tard: list[int], cbs_tard: list[int], out_file: IO):
    atlas_counter = 0
    atlas_sum = 0
    cbs_counter = 0
    cbs_sum = 0
    total = 0
    for id, (a, c) in enumerate(zip(atlas_tard, cbs_tard)):
        atlas_sum += a
        if (a > 0):
            atlas_counter += 1

        cbs_sum += a
        if (c > 0):
            cbs_counter += 1

        total += a + c

        print(f"d {id} {a + c}", file=out_file)

    print(f"a_c {atlas_counter}", file=out_file)
    print(f"a_s {atlas_sum}", file=out_file)
    print(f"c_c {cbs_counter}", file=out_file)
    print(f"c_s {cbs_sum}", file=out_file)
    print(f"total {total}", file=out_file)


def main():
    args = process_cmd_args()
    atlas_tard = parse_eval(args.atlas_input)
    cbs_tard = parse_eval(args.cbs_input)

    out_file = sys.stdout
    if (args.output != ''):
        out_file = open(args.output, 'w')

    compare(atlas_tard, cbs_tard, out_file)

    if (args.output != ''):
        out_file.close()


if __name__ == "__main__":
    main()
