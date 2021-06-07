#! /usr/bin/env python
import argparse
import numpy
import random
import sys
import time

from pathlib import Path
from typing import IO


class Job:
    job_id: int
    submission_time: int
    execution_time: int
    deadline: int

    def __init__(self, job_id: int, submission_time: int, execution_time: int, deadline: int):
        self.job_id = job_id
        self.submission_time = submission_time
        self.execution_time = execution_time
        self.deadline = deadline


class Task:
    task_id: int
    period: int
    execution_time: int
    jobs: list[Job]

    def __init__(self, task_id: int, period: int, execution_time: int):
        self.task_id = task_id
        self.period = period
        self.execution_time = execution_time
        self.jobs = []


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


def gen_tasks(num_tasks: int, num_cores: int, num_jobs: int, estimation_error: int,
              normal_gen: numpy.random.Generator):
    utilisation_left: float = num_cores
    tasks = []
    for task_id in range(num_tasks):
        period = random.randrange(2000, 10000)
        max_execution_time = int(min(period, utilisation_left * period) * 0.8)
        execution_time = random.randrange(25, max_execution_time)
        utilisation_left -= execution_time / period
        tasks.append(Task(task_id, period, execution_time))

    max_period = max([t.period for t in tasks])
    max_dl = max_period * num_jobs

    job_id = 0

    for task in tasks:
        # fill jobs to maximal deadline
        num_jobs = int(max_dl / task.period)
        current_submission = 0
        for n in range(1, num_jobs + 1):
            prev_submission = current_submission

            current_submission = int(normal_gen.normal((n-1) * task.period,
                                                       estimation_error / 100 * task.period))

            # submission has to be after previous one
            current_submission = max(current_submission, prev_submission)

            # there has to be enough time before dl
            deadline = n * task.period
            current_submission = min(current_submission, deadline - task.execution_time)

            execution_time_estimate = task.execution_time
            execution_time = int(normal_gen.normal(execution_time_estimate,
                                                   estimation_error / 100
                                                   * execution_time_estimate))

            # execution time has to be more than 20
            execution_time = max(execution_time, 20)

            task.jobs.append(Job(job_id, current_submission, execution_time, deadline))
            job_id += 1

    return tasks


def gen_atlas_workload(tasks: list[Task], config: AtlasConfig):
    print(f"c {config.num_cores}", file=config.output_file)
    print(f"f {config.cfs_factor}", file=config.output_file)

    first_in_task = True
    for task in tasks:
        for job in task.jobs:
            print(f"j {job.job_id} {job.deadline} {task.execution_time} {job.execution_time}"
                  f" {job.submission_time}",
                  file=config.output_file)
            if first_in_task:
                first_in_task = False
            else:
                print(f"d k {job.job_id} {job.job_id - 1}", file=config.output_file)
        first_in_task = True


def gen_cbs_workload(tasks: list[Task], config: CbsConfig):
    print(f"c {config.num_cores}", file=config.output_file)

    for task in tasks:
        # print server spec
        print(f"S {task.task_id} {task.execution_time} {task.period}", file=config.output_file)

        # fill jobs to maximal deadline
        for job in task.jobs:
            print(f"j {job.job_id} {job.execution_time} {job.submission_time} {task.task_id}",
                  file=config.output_file)


def main():
    args = process_cmd_args()
    cbs_config = CbsConfig(args)
    atlas_config = AtlasConfig(args)

    seed = args.seed if args.seed else time.time()
    random.seed(seed)
    normal_gen = numpy.random.default_rng(int(seed))

    tasks = gen_tasks(atlas_config.num_tasks, atlas_config.num_cores, atlas_config.num_jobs,
                      atlas_config.estimation_error, normal_gen)

    if args.both:
        gen_cbs_workload(tasks, cbs_config)
        gen_atlas_workload(tasks, atlas_config)

    elif args.cbs:
        gen_cbs_workload(tasks, cbs_config)

    else:
        gen_atlas_workload(tasks, atlas_config)


if __name__ == "__main__":
    main()
