#!/bin/bash
make

./mh_battle_royale 10 2> /dev/null
./mh_battle_royale 30 2> /dev/null
./mh_battle_royale 50 2> /dev/null

python extract.py results_MH_battle_royale/