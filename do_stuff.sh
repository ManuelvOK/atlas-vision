#! /usr/bin/env bash

for i in {1..50}
do
    ./gen_workload.py -e 10 -t 3 -j 4 -u 110 -b -o gen/"$i"

    build/main -s -o=gen/"$i"_out_atlas gen/"$i"_atlas
    ./evaluate_simulation.py -o gen/"$i"_eval_atlas gen/"$i"_out_atlas

    build/main -s -c -o=gen/"$i"_out_cbs gen/"$i"_cbs
    ./evaluate_simulation.py -o gen/"$i"_eval_cbs gen/"$i"_out_cbs

    ./compare_evaluation.py -o gen/"$i"_eval gen/"$i"_eval_atlas gen/"$i"_eval_cbs
done

