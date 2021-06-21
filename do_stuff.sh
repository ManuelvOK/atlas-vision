#! /usr/bin/env bash

exec 9>.idcache.lock

EVAL_FILE=${1:-"gen/eval"}; shift
ERROR=${1:-"10"}; shift
TASKS=${1:-"5"}; shift
JOBS=${1:-"5"}; shift
N_SIMULATIONS=${1:-"500"}; shift

#TOTAL=$((N_SIMULATIONS * 21))
#CURRENT="0"

function dostuff {
    local u=$1; shift
    local i=$1; shift
    ./gen_workload.py -e "$ERROR" -t "$TASKS" -j "$JOBS" -u "$u" -b -o gen/"$u"/"$i"

    build/main --headless -o=gen/"$u"/"$i"_out_atlas gen/"$u"/"$i"_atlas >/dev/null 2>/dev/null
    ./evaluate_simulation.py -o gen/"$u"/"$i"_eval_atlas gen/"$u"/"$i"_out_atlas

    build/main --headless -c -o=gen/"$u"/"$i"_out_cbs gen/"$u"/"$i"_cbs >/dev/null 2>/dev/null
    ./evaluate_simulation.py -o gen/"$u"/"$i"_eval_cbs gen/"$u"/"$i"_out_cbs

    local data
    data=$(./compare_evaluation.py -u "$u" gen/"$u"/"$i"_eval_atlas gen/"$u"/"$i"_eval_cbs)
    flock 9
    echo "$data" >> "$EVAL_FILE"
    #((CURRENT++))
    #echo -ne "\033[FProcessing $CURRENT of $TOTAL    \n"
    flock -u 9
}


mkdir -p gen
#echo ""
echo "utilisation,n_jobs,atlas_counter,atlas_counter/jobs,atlas_tardiness_sum,atlas_tardiness/jobs,atlas_sum,atlas/jobs,cbs_counter,cbs_counter/jobs,cbs_tardiness_sum,cbs_tardiness/jobs,cbs_sum,cbs/jobs,total_counter,total_counter/jobs,total_sum,total/jobs,total_tardiness_sum,total_tardiness/jobs,atlas_tardiness/atlas_counter,cbs_tardiness/cbs_counter,total_tardiness/total_counter" > "$EVAL_FILE"
for u in $(seq 50 5 150)
do
    mkdir -p gen/"$u"
    for i in $(seq 1 "$N_SIMULATIONS")
    do
        dostuff "$u" "$i" &
    done
done

wait
