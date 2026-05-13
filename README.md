# Post-Quantum signatures benchmark

## Purpose
This is a set of scripts used to generate the quantum signature tests described in the document:

TODO: set link to the pdf file we'll publish on our web site.

## Content Overview

* glibchooks: a custom library to pre-load used to keep track of allocated memory
* zonesign: contains files to be stored in the zonesign directory during installation as well as a fake-zone generator
* download-and-build: installation script (user-space)
* settings: various settings available to tune the benchmark
* run: base script containing the various phases of the benchmark
* run_it_all: script that runs every phase of 'run' in order
* parser/quantum-test-parser.py: a python3 script to parse the results
* reset: deletes all the downloaded and generated files in the current directory (quantum-test)

## Setup

Originally made for an Arch-linux derivative.
Others distribution should work with a bit of tuning.

Requires python, git, gcc, cmake, ninja to be installed. (pacman -Syu python git gcc cmake ninja)
Requires libssl >= 3.0 to be installed with the headers. (pacman -Syu openssl)
Requires liboqs >= 0.15 to be installed with the headers. (yay -Sy liboqs)

The "settings" files contains the default parameters along with some explanations.
By default, the bulk of the generated files will be placed in "/tmp/registry", created and owned by the current user.

Everything runs with a simple user account.

The download-and-build script:
* downloads and compiles yadifad 3.0.9 with OQS support
* compile a glibc hook to keep track of the memory usage
* generates a fake and tiny eu.zone
* creates the directory /tmp/registry
* copies a the (fake) eu.zone file into /tmp/registry
* copies a the yadifa.conf.template file into /tmp/registry

The run-it-all script:
* clears the previous generated data inside /tmp/registry
* generates keys for all the selected algorithms (inside the 'run' script)
* signs the provided eu.zone with each selected algorithm
* loads each signed zone into yadifad, then makes a few pre-defined queries so some practical measures can be made
* parses the results into a results.csv file

With the default parameters it's expected to take 216MB of space in /tmp/registry.

## Execution

To setup the benchmark:

./download-and-build

If no error occur, run the benchmark:

./run_it_all

A results.csv file will be generated in the directory of the 'run' script, as well as a few text files.
