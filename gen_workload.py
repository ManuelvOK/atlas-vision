#! /usr/bin/env python
import argparse
import numpy
import random
import sys
import time

from pathlib import Path
from typing import IO, Union


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
    output_file: IO

    def __init__(self, args: argparse.Namespace):
        self.num_cores = args.num_cores
        self.cfs_factor = args.cfs_factor
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
    output_file: IO

    def __init__(self, args: argparse.Namespace):
        self.num_cores = args.num_cores
        self.utilisation = args.utilisation
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


def is_greater_zero(value: str) -> int:
    num_value = int(value)
    if num_value < 1:
        raise argparse.ArgumentTypeError(
            f"Value has to be at least 1. {num_value} given.")
    return num_value


def is_positive(value: str) -> int:
    num_value = int(value)
    if num_value < 1:
        raise argparse.ArgumentTypeError(
            f"Value has to be positive. {num_value} given.")
    return num_value


def process_cmd_args() -> argparse.Namespace:
    aparser = argparse.ArgumentParser()
    aparser.add_argument('-n', '--num-cores', type=is_greater_zero, default=1,
                         help='number of cores')
    aparser.add_argument('-t', '--num_tasks', type=is_positive, default=5,
                         help='number of tasks to generate')
    aparser.add_argument('-j', '--num_jobs', type=is_positive, default=5,
                         help='minimum number of jobs per task')
    aparser.add_argument('-u', '--utilisation', type=is_positive, default=80,
                         help='desired overall utilisation in percent (before applying error)')
    aparser.add_argument('-l', '--length', type=is_positive, default=10000,
                         help='length of simulation. Latest deadline.')
    aparser.add_argument('-e', '--estimation-error', type=is_positive, default=5,
                         help='estimation error in percent')
    aparser.add_argument('-s', '--seed', help='Seed for the RNG')

    aparser.add_argument('-c', '--cbs', action='store_true',
                         help='generate workload for the CBS scheduler')
    aparser.add_argument('-b', '--both', action='store_true',
                         help='generate ATLAS and CBS workload with same seed into files')
    aparser.add_argument('-d', '--task-definition', default='',
                         help='print task definition to given file')
    aparser.add_argument('-o', '--output', help='output file')

    aparser.add_argument('-f', '--cfs-factor', type=is_greater_zero, default=1,
                         help='cfs factor for ATLAS')
    return aparser.parse_args()


def gen_job(job_id: int, prev_submission: int, task: Task, position_in_task: int,
            estimation_error: int, normal_gen: numpy.random.Generator) -> Union[Job, None]:
    deadline = position_in_task * task.period

    # submission mean is definitely after the previous submission
    submission_mean = (position_in_task - 1) * task.period
    if position_in_task - 1:
        submission_mean = max(submission_mean, prev_submission)

    in_bounds = False
    n_tries = 0
    for _ in range(20):
        n_tries += 1
        current_submission = int(normal_gen.normal(submission_mean,
                                                   estimation_error / 100 * task.period))
        if current_submission <= prev_submission:
            continue
        in_bounds = True
        break

    if not in_bounds:
        return None

    # if n_tries > 1:
    #     print(f"{n_tries} tries needed for submission_time", file=sys.stderr)

    # enforce that the execution time is over 20
    execution_time_estimate = task.execution_time
    in_bounds = False
    n_tries = 0
    for _ in range(20):
        n_tries += 1
        execution_time = int(normal_gen.normal(execution_time_estimate,
                                               estimation_error / 100
                                               * execution_time_estimate))
        if execution_time <= 20:
            continue
        in_bounds = True
        break

    # if n_tries > 1:
    #     print(f"{n_tries} tries needed for execution_time", file=sys.stderr)

    if not in_bounds:
        return None

    return Job(job_id, current_submission, execution_time, deadline)


def gen_tasks(num_tasks: int, num_cores: int, num_jobs: int, estimation_error: int,
              utilisation: int, simulation_length: int,
              normal_gen: numpy.random.Generator) -> list[Task]:
    task_periods = [random.randrange(500, 2000) for _ in range(num_tasks)]
    max_period = max(task_periods)
    maximal_length = max_period * num_jobs
    task_periods_norm = [period / maximal_length for period in task_periods]

    task_lengths = [random.randrange(500, 2000) for _ in range(num_tasks)]
    total_length = sum(task_lengths)
    task_lengths_norm = [length / total_length * utilisation / 100 for length in task_lengths]

    if max(task_lengths_norm) > 1:
        # print("There are Tasks with utilisation > 1", file=sys.stderr)
        return []

    tasks = []
    for task_id, (period_n, length_n) in enumerate(zip(task_periods_norm, task_lengths_norm)):
        period = int(period_n * simulation_length)
        execution_time = int(length_n * period)
        tasks.append(Task(task_id, period, execution_time))

    max_period = max([t.period for t in tasks])
    max_dl = max_period * num_jobs

    job_id = 0

    for task in tasks:
        # fill jobs to maximal deadline
        num_jobs = int(max_dl / task.period)
        current_submission = -task.period
        for position_in_task in range(1, num_jobs + 1):
            job = gen_job(job_id, current_submission, task, position_in_task, estimation_error,
                          normal_gen)
            if not job:
                return []
            current_submission = job.submission_time
            task.jobs.append(job)
            job_id += 1

    return tasks


def gen_atlas_workload(tasks: list[Task], config: AtlasConfig) -> None:
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


def gen_cbs_workload(tasks: list[Task], config: CbsConfig) -> None:
    print(f"c {config.num_cores}", file=config.output_file)

    for task in tasks:
        # print server spec
        print(f"S {task.task_id} {task.execution_time} {task.period}", file=config.output_file)

        # fill jobs to maximal deadline
        for job in task.jobs:
            print(f"j {job.job_id} {job.execution_time} {job.submission_time} {task.task_id}",
                  file=config.output_file)


def write_tasks(tasks: list[Task], file: IO) -> None:
    for task in tasks:
        print(f"t {task.task_id} {task.period} {task.execution_time}", file=file)
        for job in task.jobs:
            print(f"j {job.job_id} {task.task_id} {job.submission_time} {job.execution_time}"
                  f" {job.deadline}", file=file)


def main() -> None:
    args = process_cmd_args()
    cbs_config = CbsConfig(args)
    atlas_config = AtlasConfig(args)

    seed = args.seed if args.seed else time.time()
    random.seed(seed)
    normal_gen = numpy.random.default_rng(int(seed))

    tasks = []
    n_tries = 0
    for _ in range(50):
        n_tries += 1
        tasks = gen_tasks(args.num_tasks, args.num_cores, args.num_jobs, args.estimation_error,
                          args.utilisation, args.length, normal_gen)
        if tasks:
            break

    if not tasks:
        print(f"Tried 50 times but could not generate an appropriate scenario. Seed: {seed}",
              file=sys.stderr)
        exit(1)

    # if n_tries > 1:
    #     print(f"{n_tries} tries needed for gen_tasks()", file=sys.stderr)

    if args.task_definition != '':
        with open(args.task_definition, 'w') as f:
            write_tasks(tasks, f)

    if args.both:
        gen_cbs_workload(tasks, cbs_config)
        gen_atlas_workload(tasks, atlas_config)

    elif args.cbs:
        gen_cbs_workload(tasks, cbs_config)

    else:
        gen_atlas_workload(tasks, atlas_config)


if __name__ == "__main__":
    main()
