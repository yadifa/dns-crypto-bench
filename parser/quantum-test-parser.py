#!/usr/bin/env python3
#
# simple parser for the produced data
#
# quantum-test-parser.py [TLD] [TARGET_FILE_NAME] [QUANTUM_DIR]
#
# expects both parameters or none
#
# By default, TLD=eu TARGET_FILE_NAME=eu.zone QUANTUM_DIR=/tmp/registry/quantum
#

import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path

DEBUG = 0

TLD="eu"
ZONEFILENAME = f"{TLD}.zone"
BASEDIR = "/tmp/registry/quantum"

if len(sys.argv) != 1:
    if len(sys.argv) == 4:
        TLD = sys.argv[1]
        ZONEFILENAME = sys.argv[2]
        BASEDIR = sys.argv[3]
    else:
        print(f"""{sys.argv[0]} TLD ZONE-FILE_NAME QUANTUM_DIR
        
e.g.

  {sys.argv[0]} {TLD} {ZONEFILENAME} {BASEDIR}""")
        exit(1)

# don't
# print(f"{sys.argv[0]} {TLD} {ZONEFILENAME} {BASEDIR}")

ALGORITHMS_NAMES = []
ALGORITHMS = dict()
QUERIES_NAMES=['SOA', 'NS', 'DNSKEY', 'ANY', 'ANY-delegation', 'AXFR']
ANSWERS = dict()


@dataclass
class PublicKey:
    filename: str
    flags: int
    protocol: int # 3
    algorithm: int
    encoded: str
    size: int

    def __init__(self, filename):
        with open(filename) as f:
            while True:
                line = f.readline()
                if line is None or len(line) == 0:
                    break
                if line[0] == ';':
                    continue
                m = re.search(r'^([A-Za-z0-9_.-]+).*\sDNSKEY\s+(\d+)\s+(\d+)\s+(\d+)\s+(.*)', line)
                if m is not None:
                    self.filename = m.group(1)
                    self.flags = int(m.group(2))
                    self.protocol = int(m.group(3))
                    self.algorithm = int(m.group(4))
                    self.encoded = m.group(5).replace(' ','')
                    self.size = int(4 + (len(self.encoded) * 5) / 8)
                    break


@dataclass
class Answer:
    name: str
    time: int = 0
    size: int = 0

    def __init__(self, name, answer_file):
        try:
            self.name = name
            with open(answer_file) as f:
                answer_size = os.stat(answer_file).st_size
                close_to_the_end = max(answer_size - 4096, 0)
                f.seek(close_to_the_end)
                while True:
                    line = f.readline()
                    if line is None or len(line) == 0:
                        break
                    if line[0] != ';':
                        continue
                    m = re.search(r'^;; Query time: (\d+)', line)
                    if m is not None:
                        self.time = int(m.group(1))
                        continue
                    m = re.search(r'^;; XFR size:.*bytes (\d+)', line)
                    if m is not None:
                        self.size = int(m.group(1))
                        continue
                    m = re.search(r'^;; MSG SIZE.*: (\d+)', line)
                    if m is not None:
                        self.size = int(m.group(1))
                        continue
        except FileNotFoundError as ex:
            pass


@dataclass
class Algorithm:
    name: str
    id: int = 0
    public_key_size: int = 0
    zone_file_size: int = 0
    axfr_file_size: int = 0
    sign_zone_memory: int = 0
    sign_zone_time: float = 0
    ksk: PublicKey = None
    zsk: PublicKey = None

    def __init__(self, name):
        self.name = name

    def update(self):
        size = 0
        div = 0
        if self.ksk is not None:
            size += self.ksk.size
            div += 1
            self.id = self.ksk.algorithm
        if self.zsk is not None:
            size += self.zsk.size
            div += 1
        if div > 0:
            self.public_key_size = int(size / div)


def read_algorithms():
    algorithm_names_list = os.listdir(BASEDIR)
    for entry in algorithm_names_list:
        if os.path.isdir(os.path.join(BASEDIR, entry)):
            ALGORITHMS_NAMES.append(entry)
            ALGORITHMS[entry] = Algorithm(entry)
    ALGORITHMS_NAMES.sort()
    if DEBUG:
        print(ALGORITHMS_NAMES, file=sys.stderr)


def parse_algorithms():
    for a in ALGORITHMS_NAMES:
        zone_file = os.path.join(BASEDIR, ZONEFILENAME + '.' + a)
        if not os.path.exists(zone_file):
            print(f"skipping algorithm {a} ({zone_file} does not exist)", file=sys.stderr)
            continue
        ALGORITHMS[a].zone_file_size = os.stat(zone_file).st_size
        
        algorithm_dir = os.path.join(BASEDIR, a)

        axfr_file_matches = list(Path(algorithm_dir).rglob(f"{TLD}*.axfr"))
        if len(axfr_file_matches) != 1:
            print(f"algorithm {a}: didn't find exactly one .axfr file for '{TLD}' ({axfr_file_matches})")
            exit(1)

        axfr_file_name = axfr_file_matches[0]

        axfr_file = os.path.join(algorithm_dir, axfr_file_name)
        if not os.path.exists(axfr_file):
            print(f"skipping algorithm {a} ({axfr_file} does not exist)", file=sys.stderr)
            continue
        ALGORITHMS[a].axfr_file_size = os.stat(axfr_file).st_size

        algorithm_dir_list = os.listdir(algorithm_dir)
        for entry in algorithm_dir_list:
            m = re.search(r'K.*\.key', entry)
            if m is not None:
                key_file = os.path.join(algorithm_dir, entry)
                k = PublicKey(key_file)
                if k.flags == 257:
                    ALGORITHMS[a].ksk = k
                elif k.flags == 256:
                    ALGORITHMS[a].zsk = k
                else:
                    raise Exception(f"Unexpected flags value {k.flags}")
                ALGORITHMS[a].update()


def parse_zonesign_hook():
    for a in ALGORITHMS_NAMES:
        zonesign_hook_file = os.path.join(BASEDIR, a, 'zonesign-hook.txt')
        if not os.path.exists(zonesign_hook_file):
            continue
        with open(zonesign_hook_file) as f:
            while True:
                line = f.readline()
                if line is None or len(line) == 0:
                    break
                m = re.match(r'^timing: start=\d+ stopped=\d+ duration=\d+ duration_seconds=([0-9.]+)', line)
                if m is not None:
                    ALGORITHMS[a].sign_zone_time = float(m.group(1))
                    continue
                m = re.match(r'malloc: count: total=\d+ peak=\d+ current=\d+ memory: total=\d+ peak=\d+ current=(\d+)', line)
                if m is not None:
                    ALGORITHMS[a].sign_zone_memory = int(m.group(1))


def parse_queries():
    for a in ALGORITHMS_NAMES:
        ANSWERS[a] = dict()
        for q in QUERIES_NAMES:
            answer_file = os.path.join(BASEDIR, a, f'{q}.txt')
            answer = Answer(q, answer_file)
            ANSWERS[a][q] = answer


def print_algorithms():
    for a in ALGORITHMS_NAMES:
        alg = ALGORITHMS[a]
        print(alg, file=sys.stderr)


def print_parsable():
    print("Name;Id;Public Key Size;Zone File Size;AXFR File Size;Sign Zone RAM; Sign Zone Time")
    for a in ALGORITHMS_NAMES:
        alg = ALGORITHMS[a]
        print(f"{alg.name};{alg.id};{alg.public_key_size};{alg.zone_file_size};{alg.axfr_file_size};{alg.sign_zone_memory};{int(alg.sign_zone_time)}")


def print_queries():
    print("Queries sizes (bytes)")
    print("")
    print("Algorithm", end=';')
    for q in QUERIES_NAMES:
        print(f"{q}", end=';')
    print("")
    for a in ALGORITHMS_NAMES:
        print(a, end=';')
        for q in QUERIES_NAMES:
            print(ANSWERS[a][q].size, end=';')
        print("")
    print("")
    print("Queries times (ms)")
    print("")
    print("Algorithm", end=';')
    for q in QUERIES_NAMES:
        print(f"{q}", end=';')
    print("")
    for a in ALGORITHMS_NAMES:
        print(a, end=';')
        for q in QUERIES_NAMES:
            print(ANSWERS[a][q].time, end=';')
        print("")


def main():
    read_algorithms()
    parse_algorithms()
    parse_zonesign_hook()
    parse_queries()
    if DEBUG:
        print_algorithms()
    print_parsable()
    print_queries()


if __name__ == '__main__':
    main()
