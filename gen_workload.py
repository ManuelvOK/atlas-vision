#! /usr/bin/env python
import argparse
import random
import sys


class AtlasConfig:
    num_cores: int
    cfs_factor: int
    num_tasks: int
    num_jobs: int
    estimation_error: int

    def __init__(self, args: argparse.Namespace):
        self.num_cores = args.num_cores
        self.cfs_factor = args.cfs_factor
        self.num_tasks = args.num_tasks
        self.num_jobs = args.num_jobs
        self.estimation_error = args.estimation_error


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


def gen_atlas_workload(config: AtlasConfig):
    print(f"c {config.num_cores}")
    print(f"f {config.cfs_factor}")

    utilisation_left: float = config.num_cores
    job_id = 0
    first_in_task = True
    tasks = []
    for _ in range(config.num_tasks):
        period = random.randrange(50, 10000)
        max_execution_time = int(min(period, utilisation_left * period) * 0.8)
        execution_time = random.randrange(25, max_execution_time)
        utilisation_left -= execution_time / period
        task = {"period": period, "execution_time": execution_time}
        tasks.append(task)

    max_period = max([task["period"] for task in tasks])
    max_dl = max_period * config.num_jobs

    for task in tasks:
        num_jobs = int(max_dl / task["period"])
        print(f"Task: p={task['period']} t={task['execution_time']} n={num_jobs}", file=sys.stderr)
        for n in range(1, num_jobs + 1):
            error = 1 + random.randrange(-config.estimation_error, config.estimation_error) / 100
            submission = (n-1) * task["period"] * error
            deadline = n * task["period"]
            execution_time_estimate = task["execution_time"]
            error = 1 + random.randrange(-config.estimation_error, config.estimation_error) / 100
            print(f"error: {error}", file=sys.stderr)
            execution_time = int(execution_time_estimate * error)
            print(f"j {job_id} {deadline} {execution_time_estimate} {execution_time} {submission}")
            if first_in_task:
                first_in_task = False
            else:
                print(f"d k {job_id} {job_id - 1}")
            job_id += 1
        first_in_task = True


def gen_cbs_workload():
    raise NotImplementedError()


def main():
    args = process_cmd_args()
    if args.cbs:
        gen_cbs_workload(CbsConfig(args))

    gen_atlas_workload(AtlasConfig(args))


if __name__ == "__main__":
    main()
