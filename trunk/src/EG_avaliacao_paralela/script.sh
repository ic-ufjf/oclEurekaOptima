#!/bin/bash

make -B
make media -B

mkdir  experiments/somatorio/execucoes_

#/EG
#	cd ../..

#--------------------------------------------------------
for n in {11..15}
    do

    echo "Execução ge - somatorio^2 com 10^5 registros - $n variável(eis)"

    touch experiments/somatorio/execucoes_/$n.txt
    echo "" >  experiments/somatorio/execucoes_/$n.txt

    for i in {1..5}
    do
       echo "Execucao - $i"
       ./eg --database="problems/somatorio/$n.txt" --grammar="grammars/g$n.txt" >> experiments/somatorio/execucoes_/$n.txt
    done

done
