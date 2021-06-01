#!/bin/fish

make

set archivos *.txt

rm -f ./results_MH_battle_royale/$archivos

for i in (seq 1 10)
    ./mh_battle_royale 10 2> /dev/null
    ./mh_battle_royale 30 2> /dev/null
end

python extract.py results_MH_battle_royale/