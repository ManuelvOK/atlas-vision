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
    aparser.add_argument('-u', '--utilisation', type=int,
                         help='scenarios utilisation')
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


def compare(atlas_tard: list[int], cbs_tard: list[int], utilisation: int, out_file: IO):
    atlas_counter = 0
    atlas_tard_sum = 0
    atlas_sum = 0

    cbs_counter = 0
    cbs_tard_sum = 0
    cbs_sum = 0

    total_counter = 0
    total_sum = 0
    for id, (a, c) in enumerate(zip(atlas_tard, cbs_tard)):
        atlas_sum += a
        if (a > 0):
            atlas_tard_sum += a
            atlas_counter += 1
            total_counter += 1

        cbs_sum += c
        if (c > 0):
            cbs_tard_sum += c
            cbs_counter += 1
            total_counter -= 1

        total_sum += a - c

        # print(f"d {id} {a - c}", file=out_file)

    n_jobs = len(atlas_tard)
    total_tard_sum = atlas_tard_sum - cbs_tard_sum

    print(f"{utilisation},{n_jobs},{atlas_counter},{atlas_counter / n_jobs},{atlas_tard_sum},"
          f"{atlas_tard_sum / n_jobs},{atlas_sum},{atlas_sum / n_jobs},{cbs_counter},"
          f"{cbs_counter / n_jobs},{cbs_tard_sum},{cbs_tard_sum / n_jobs},{cbs_sum},"
          f"{cbs_sum / n_jobs},{total_counter},{total_counter / n_jobs},{total_sum},"
          f"{total_sum / n_jobs},{total_tard_sum},{total_tard_sum / n_jobs}", file=out_file)


def main():
    args = process_cmd_args()
    atlas_tard = parse_eval(args.atlas_input)
    cbs_tard = parse_eval(args.cbs_input)

    out_file = sys.stdout
    if (args.output and args.output != ''):
        out_file = open(args.output, 'a')

    compare(atlas_tard, cbs_tard, args.utilisation, out_file)

    if (args.output != ''):
        out_file.close()


if __name__ == "__main__":
    main()
