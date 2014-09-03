#!/bin/bash

make -B
make media -B

mkdir  experiments/somatorio/execucoes3

#/EG
#	cd ../..

#--------------------------------------------------------
for n in {7..10}
    do

    echo "Execução ge - somatorio^2 com 10^5 registros - $n variável(eis)"

   # touch experiments/somatorio/execucoes3/$n.txt
   # echo "" >  experiments/somatorio/execucoes3/$n.txt

    for i in {1..1}
    do
       echo "Execucao - $i"
       ./eg --database="problems/somatorio/$n.txt" --grammar="grammars/g$n.txt" >> experiments/somatorio/execucoes3/$n.txt
    done

done
