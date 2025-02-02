#!/bin/bash

source ../sequential_benchs/scripts/common.sh

data_name=kyte

states=4
lookupsize=0
cpus=64
iterations=100
use_repeats=0
update_repeats=1
debugger=
randomized=0
seed=2
#debugger=valgrind
#debugger="gdb --args "

go="mpirun -np $cpus main full_tree_traversal $path_data/$data_name/unrooted.newick $path_data/$data_name/$data_name.phy $path_data/$data_name/$data_name.part $states $use_repeats $update_repeats $lookupsize $iterations $randomized $seed" 



echo $debugger $go
$debugger $go
