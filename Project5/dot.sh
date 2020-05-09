#!/bin/bash
thing(){
    echo -e "Trials\tMegaTrials/Second\tProbability"
    for nt in 16384 32768 65536 131072 262144 524288 1048576
    do
        bs=$(($nt / 1024))
        nvcc -DNUMTRIALS=$nt -DBLOCKSIZE=$bs -o prog prog.cu
        ./prog
    done
}

thing > results.txt
