#!/bin/bash

make -B
make media -B

mkdir execucoes

touch execucoes/time.txt 
printf "" > execucoes/time.txt 

#--------------------------------------------------------
echo "Execução ge - P4(x) com 10 registros"

mkdir execucoes/10
touch execucoes/10/time.txt
printf "" > execucoes/10/time.txt 

for i in {1..50}
do
   echo "Execucao - $i"
   touch  execucoes/10/logExecucao$i.txt
   ( /usr/bin/time  -f "%e" ./eg --database="problems/p4_10.txt") > execucoes/10/logExecucao$i.txt  2>> execucoes/10/time.txt

done

printf "10\t" >> execucoes/time.txt
./media "execucoes/10/time.txt" >> execucoes/time.txt
printf "\n" >> execucoes/time.txt

#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^2 registros"

mkdir execucoes/10^2
touch execucoes/10^2/time.txt
printf "" > execucoes/10^2/time.txt 

for i in {1..50}
do
   echo "Execucao - $i"
   touch  execucoes/10^2/logExecucao$i.txt
   ( /usr/bin/time  -f "%e" ./eg --database="problems/p4_10^2.txt") > execucoes/10^2/logExecucao$i.txt  2>> execucoes/10^2/time.txt

done

printf "100\t" >> execucoes/time.txt
./media "execucoes/10^2/time.txt" >> execucoes/time.txt
printf "\n" >> execucoes/time.txt

#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^3 registros"

mkdir execucoes/10^3
touch execucoes/10^3/time.txt
printf "" > execucoes/10^3/time.txt 

for i in {1..50}
do
   echo "Execucao - $i"
   touch  execucoes/10^3/logExecucao$i.txt
   ( /usr/bin/time  -f "%e" ./eg --database="problems/p4_10^3.txt") > execucoes/10^3/logExecucao$i.txt  2>> execucoes/10^3/time.txt

done

printf "1000\t" >> execucoes/time.txt
./media "execucoes/10^3/time.txt" >> execucoes/time.txt
printf "\n" >> execucoes/time.txt


#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^4 registros"

mkdir execucoes/10^4
touch execucoes/10^4/time.txt
printf "" > execucoes/10^4/time.txt 

for i in {1..50}
do
   echo "Execucao - $i"
   touch  execucoes/10^4/logExecucao$i.txt
   ( /usr/bin/time  -f "%e" ./eg --database="problems/p4_10^4.txt") > execucoes/10^4/logExecucao$i.txt  2>> execucoes/10^4/time.txt

done

printf "10000\t" >> execucoes/time.txt
./media "execucoes/10^4/time.txt" >> execucoes/time.txt
printf "\n" >> execucoes/time.txt

#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^5 registros"

mkdir execucoes/10^5
touch execucoes/10^5/time.txt
printf "" > execucoes/10^5/time.txt 

for i in {1..50}
do
   echo "Execucao - $i"
   touch  execucoes/10^5/logExecucao$i.txt
   ( /usr/bin/time  -f "%e" ./eg --database="problems/p4_10^5.txt") > execucoes/10^5/logExecucao$i.txt  2>> execucoes/10^5/time.txt

done

printf "100000\t" >> execucoes/time.txt
./media "execucoes/10^5/time.txt" >> execucoes/time.txt
printf "\n" >> execucoes/time.txt

#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^6 registros"

mkdir execucoes/10^6
touch execucoes/10^6/time.txt
printf "" > execucoes/10^6/time.txt 

for i in {1..50}
do
   echo "Execucao - $i"
   touch  execucoes/10^6/logExecucao$i.txt
   ( /usr/bin/time  -f "%e" ./eg --database="problems/p4_10^6.txt") > execucoes/10^6/logExecucao$i.txt  2>> execucoes/10^6/time.txt

done

printf "1000000\t" >> execucoes/time.txt
./media "execucoes/10^6/time.txt" >> execucoes/time.txt
printf "\n" >> execucoes/time.txt

mkdir Experimento

#Script para plotar o gráfico
gnuplot scripts_grafico.gnu
eog Experimento/grafico.png



