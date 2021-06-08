#! /usr/bin/env python
import argparse
import sys

from typing import IO


class Schedule:
    id: int
    job_id: int
    core: int
    scheduler: str
    submission_time: int
    begin: int
    execution_time: int

    def __init__(self, spec: str):
        split = spec.split(' ')
        self.id = int(split[1])
        self.job_id = int(split[2])
        self.core = int(split[3])
        self.scheduler = split[4]
        self.submission_time = int(split[5])
        self.begin = int(split[6])
        self.execution_time = int(split[7])


class Job:
    id: int
    deadline: int
    execution_time_estimate: int
    execution_time: int
    submission_time: int
    schedules: list[Schedule]

    def __init__(self, spec: str):
        split = spec.split(' ')
        self.id = int(split[1])
        self.deadline = int(split[2])
        self.execution_time_estimate = int(split[3])
        self.execution_time = int(split[4])
        self.submission_time = int(split[5])
        self.schedules = []


def process_cmd_args():
    aparser = argparse.ArgumentParser()
    aparser.add_argument('input', metavar='INPUT', type=str,
                         help='input file')
    aparser.add_argument('-o', '--output', help='output file')
    return aparser.parse_args()


def parse_file(input_file: str):
    jobs: dict[int, Job]
    jobs = {}
    schedules = []
    with open(input_file) as f:
        for line in f:
            if not len(line):
                continue
            if line[0] == 'j':
                job = Job(line)
                jobs[job.id] = job
            if line[0] == 's':
                schedules.append(Schedule(line))

    for schedule in schedules:
        jobs[schedule.job_id].schedules.append(schedule)

    return jobs


def evaluate(jobs: dict[int, Job], out_file: IO):
    for job in jobs.values():
        finishing_time = max([s.begin + s.execution_time for s in job.schedules])

        out_file.write(f"t {job.id} {finishing_time - job.deadline}\n")


def main():
    args = process_cmd_args()
    jobs = parse_file(args.input)

    out_file = sys.stdout
    if (args.output != ''):
        out_file = open(args.output, 'w')

    evaluate(jobs, out_file)

    if (args.output != ''):
        out_file.close()


if __name__ == "__main__":
    main()
