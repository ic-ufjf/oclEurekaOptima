#!/bin/bash

make -B
make media -B

mkdir  experiments/somatorio/execucoes

#/EG
#	cd ../..

#--------------------------------------------------------
for n in {11..15}
    do

    echo "Execução ge - somatorio^2 com 10^5 registros - $n variável(eis)"

    touch experiments/somatorio/execucoes/$n.txt
    #echo "" >  experiments/somatorio/execucoes/$n.txt

    for i in {1..2}
    do
       echo "Execucao - $i"
       ./eg --database="problems/somatorio/$n.txt" --grammar="grammars/g$n.txt" >> experiments/somatorio/execucoes/$n.txt
    done

done
