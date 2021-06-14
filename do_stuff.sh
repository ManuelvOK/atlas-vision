#! /usr/bin/env bash

EVAL_FILE=${1:-"gen/eval"}; shift
ERROR=${1:-"10"}; shift
TASKS=${1:-"5"}; shift
JOBS=${1:-"5"}; shift
N_SIMULATIONS=${1:-"50"}; shift

function dostuff {
    local u=$1; shift
    local i=$1; shift
    ./gen_workload.py -e "$ERROR" -t "$TASKS" -j "$JOBS" -u "$u" -b -o gen/"$u"/"$i"

    build/main --headless -o=gen/"$u"/"$i"_out_atlas gen/"$u"/"$i"_atlas >/dev/null 2>/dev/null
    ./evaluate_simulation.py -o gen/"$u"/"$i"_eval_atlas gen/"$u"/"$i"_out_atlas

    build/main --headless -c -o=gen/"$u"/"$i"_out_cbs gen/"$u"/"$i"_cbs >/dev/null 2>/dev/null
    ./evaluate_simulation.py -o gen/"$u"/"$i"_eval_cbs gen/"$u"/"$i"_out_cbs

    ./compare_evaluation.py -u "$u" -o "$EVAL_FILE" gen/"$u"/"$i"_eval_atlas gen/"$u"/"$i"_eval_cbs
    # echo -ne "\033[FProcessing $i of $TOTAL    \n"
}

mkdir -p gen
# echo ""
echo "utilisation,n_jobs,atlas_counter,atlas_counter/jobs,atlas_tardiness_sum,atlas_tardiness/jobs,atlas_sum,atlas/jobs, cbs_counter,cbs_counter/jobs,cbs_tardiness_sum,cbs_tardiness/jobs,cbs_sum,cbs/jobs,total_counter,total_counter/jobs,total_sum,total/jobs,total_tardiness_sum,total_tardiness/jobs" > "$EVAL_FILE"
for u in $(seq 50 5 150)
do
    mkdir -p gen/"$u"
    for i in $(seq 1 "$N_SIMULATIONS")
    do
        dostuff "$u" "$i" &
    done
done
