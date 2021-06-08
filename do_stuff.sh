#! /usr/bin/env bash


./gen_workload.py -e 10 -t 3 -j 4 -u 110 -b -o gen/foo

build/main -s -o=gen/foo_out_atlas gen/foo_atlas
./evaluate_simulation.py -o gen/foo_eval_atlas gen/foo_out_atlas

build/main -s -c -o=gen/foo_out_cbs gen/foo_cbs
./evaluate_simulation.py -o gen/foo_eval_cbs gen/foo_out_cbs

./compare_evaluation.py -o gen/foo_eval gen/foo_eval_atlas gen/foo_eval_cbs
