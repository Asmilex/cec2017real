#!/bin/bash
make

rm ./results_MH_battle_royale/*

for i in {1..5}
do
    ./mh_battle_royale 10 2> /dev/null
    ./mh_battle_royale 30 2> /dev/null
done

python extract.py results_MH_battle_royale/