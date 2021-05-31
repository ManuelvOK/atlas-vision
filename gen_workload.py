#! /usr/bin/env python
import argparse
import random
import sys
import time

from pathlib import Path
from typing import IO


class AtlasConfig:
    num_cores: int
    cfs_factor: int
    num_tasks: int
    num_jobs: int
    estimation_error: int
    output_file: IO

    def __init__(self, args: argparse.Namespace):
        self.num_cores = args.num_cores
        self.cfs_factor = args.cfs_factor
        self.num_tasks = args.num_tasks
        self.num_jobs = args.num_jobs
        self.estimation_error = args.estimation_error
        if not args.output:
            self.output_file = sys.stdout
        elif args.both:
            path = Path(args.output)
            self.output_file = open(f"{path.parent}/{path.stem}_atlas{path.suffix}", 'w')
        else:
            self.output_file = open(args.output, 'w')

    def __del__(self):
        if self.output_file != sys.stdout:
            self.output_file.close()


class CbsConfig:
    num_cores: int
    num_tasks: int
    num_jobs: int
    estimation_error: int
    output_file: IO

    def __init__(self, args: argparse.Namespace):
        self.num_cores = args.num_cores
        self.num_tasks = args.num_tasks
        self.num_jobs = args.num_jobs
        self.estimation_error = args.estimation_error
        if not args.output:
            self.output_file = sys.stdout
        elif args.both:
            path = Path(args.output)
            self.output_file = open(f"{path.parent}/{path.stem}_cbs{path.suffix}", 'w')
        else:
            self.output_file = open(args.output, 'w')

    def __del__(self):
        if self.output_file != sys.stdout:
            self.output_file.close()


def is_greater_zero(value):
    num_value = int(value)
    if num_value < 1:
        raise argparse.ArgumentTypeError(
            f"Value has to be at least 1. {num_value} given.")
    return num_value


def is_positive(value):
    num_value = int(value)
    if num_value < 1:
        raise argparse.ArgumentTypeError(
            f"Value has to be positive. {num_value} given.")
    return num_value


def process_cmd_args():
    aparser = argparse.ArgumentParser()
    aparser.add_argument('-c', '--cbs', action='store_true',
                         help='generate workload for the CBS scheduler')
    aparser.add_argument('-n', '--num-cores', type=is_greater_zero, default=1,
                         help='number of cores')
    aparser.add_argument('-s', '--seed', help='Seed for the RNG')

    aparser.add_argument('-b', '--both', action='store_true',
                         help='generate ATLAS and CBS workload with same seed into files')
    aparser.add_argument('-o', '--output', help='output file')

    aparser.add_argument('-f', '--cfs-factor', type=is_greater_zero, default=1,
                         help='cfs factor for ATLAS')
    aparser.add_argument('-t', '--num_tasks', type=is_positive, default=5,
                         help='number of tasks for ATLAS')
    aparser.add_argument('-j', '--num_jobs', type=is_positive, default=5,
                         help='number of jobs for ATLAS')
    aparser.add_argument('-e', '--estimation-error', type=is_positive,
                         default=5,
                         help='estimation error in percent for ATLAS')
    return aparser.parse_args()


def gen_tasks(num_tasks: int, num_cores: int):
    utilisation_left: float = num_cores
    tasks = []
    for _ in range(num_tasks):
        period = random.randrange(50, 10000)
        max_execution_time = int(min(period, utilisation_left * period) * 0.8)
        execution_time = random.randrange(25, max_execution_time)
        utilisation_left -= execution_time / period
        task = {"period": period, "execution_time": execution_time}
        tasks.append(task)
    return tasks


def gen_atlas_workload(config: AtlasConfig):
    print(f"c {config.num_cores}", file=config.output_file)
    print(f"f {config.cfs_factor}", file=config.output_file)

    tasks = gen_tasks(config.num_tasks, config.num_cores)

    max_period = max([task["period"] for task in tasks])
    max_dl = max_period * config.num_jobs

    job_id = 0
    first_in_task = True
    for task in tasks:
        # fill jobs to maximal deadline
        num_jobs = int(max_dl / task["period"])
        submission = 0
        for n in range(1, num_jobs + 1):
            submission_error = random.randrange(-config.estimation_error,
                                                config.estimation_error) / 100
            # submission has to be after previous one
            submission = max(submission, int((n-1) * task["period"]
                                             - (task["period"] * submission_error)))
            deadline = n * task["period"]
            execution_time_estimate = task["execution_time"]
            estimation_error = 1 + random.randrange(-config.estimation_error,
                                                    config.estimation_error) / 100
            execution_time = int(execution_time_estimate * estimation_error)
            print(f"j {job_id} {deadline} {execution_time_estimate} {execution_time} {submission}",
                  file=config.output_file)
            if first_in_task:
                first_in_task = False
            else:
                print(f"d k {job_id} {job_id - 1}", file=config.output_file)
            job_id += 1
        first_in_task = True


def gen_cbs_workload(config: CbsConfig):
    print(f"c {config.num_cores}", file=config.output_file)
    tasks = gen_tasks(config.num_tasks, config.num_cores)

    max_period = max([task["period"] for task in tasks])
    max_dl = max_period * config.num_jobs

    job_id = 0
    for task_id, task in enumerate(tasks):
        # print server spec
        print(f"S {task_id} {task['execution_time']} {task['period']}", file=config.output_file)
        # fill jobs to maximal deadline
        num_jobs = int(max_dl / task["period"])
        submission = 0
        for n in range(1, num_jobs + 1):
            submission_error = random.randrange(-config.estimation_error,
                                                config.estimation_error) / 100
            # submission has to be after previous one
            submission = max(submission, int((n-1) * task["period"]
                                             - (task["period"] * submission_error)))
            deadline = n * task["period"]
            execution_time_estimate = task["execution_time"]
            estimation_error = 1 + random.randrange(-config.estimation_error,
                                                    config.estimation_error) / 100
            execution_time = int(execution_time_estimate * estimation_error)
            print(f"j {job_id} {execution_time} {submission} {task_id}", file=config.output_file)
            job_id += 1


def main():
    args = process_cmd_args()
    seed = args.seed if args.seed else time.time()
    random.seed(seed)

    if args.both:
        gen_cbs_workload(CbsConfig(args))

        random.seed(seed)
        gen_atlas_workload(AtlasConfig(args))

    elif args.cbs:
        gen_cbs_workload(CbsConfig(args))

    else:
        gen_atlas_workload(AtlasConfig(args))


if __name__ == "__main__":
    main()
