#! /usr/bin/env bash

TOTAL=50
echo ""
for i in $(seq 1 $TOTAL)
do
    ./gen_workload.py -e 10 -t 3 -j 4 -u 110 -b -o gen/"$i"

    build/main --headless -o=gen/"$i"_out_atlas gen/"$i"_atlas >/dev/null 2>/dev/null
    ./evaluate_simulation.py -o gen/"$i"_eval_atlas gen/"$i"_out_atlas

    build/main --headless -c -o=gen/"$i"_out_cbs gen/"$i"_cbs >/dev/null 2>/dev/null
    ./evaluate_simulation.py -o gen/"$i"_eval_cbs gen/"$i"_out_cbs

    ./compare_evaluation.py -o gen/"$i"_eval gen/"$i"_eval_atlas gen/"$i"_eval_cbs
    echo -ne "\033[FProcessing $i of $TOTAL    \n"
done
