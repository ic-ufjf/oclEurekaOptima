#!/bin/bash

make -B
make media -B

mkdir Experimento/SelecaoAleatoria
mkdir Experimento/SelecaoAleatoria/execucoes

declare -i PCORES
PCORES=4

touch Experimento/SelecaoAleatoria/execucoes/time.txt 
echo -e "N\tAvaliacao\tTotal" > Experimento/SelecaoAleatoria/execucoes/time.txt 

#--------------------------------------------------------
echo "Execução ge - P4(x) com 10 registros"

mkdir Experimento/SelecaoAleatoria/execucoes/10
touch Experimento/SelecaoAleatoria/execucoes/10/time.txt
printf "" > Experimento/SelecaoAleatoria/execucoes/10/time.txt 

for i in {1..30}
do
   echo "Execucao - $i"
   #touch  Experimento/SelecaoAleatoria/execucoes/10/logExecucao$i.txt
   ./eg --database="../../problems/p4_10.txt" --cores=$PCORES >> Experimento/SelecaoAleatoria/execucoes/10/time.txt

done

printf "10\t" >> Experimento/SelecaoAleatoria/execucoes/time.txt
./media "Experimento/SelecaoAleatoria/execucoes/10/time.txt" >> Experimento/SelecaoAleatoria/execucoes/time.txt
printf "\n" >> Experimento/SelecaoAleatoria/execucoes/time.txt

#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^2 registros"

mkdir Experimento/SelecaoAleatoria/execucoes/10^2
touch Experimento/SelecaoAleatoria/execucoes/10^2/time.txt
printf "" > Experimento/SelecaoAleatoria/execucoes/10^2/time.txt 

for i in {1..30}
do
   echo "Execucao - $i"
   #touch  Experimento/SelecaoAleatoria/execucoes/10^2/logExecucao$i.txt
   ./eg --database="../../problems/p4_10^2.txt"  --cores=$PCORES >> Experimento/SelecaoAleatoria/execucoes/10^2/time.txt

done

printf "100\t" >> Experimento/SelecaoAleatoria/execucoes/time.txt
./media "Experimento/SelecaoAleatoria/execucoes/10^2/time.txt" >> Experimento/SelecaoAleatoria/execucoes/time.txt
printf "\n" >> Experimento/SelecaoAleatoria/execucoes/time.txt

#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^3 registros"

mkdir Experimento/SelecaoAleatoria/execucoes/10^3
touch Experimento/SelecaoAleatoria/execucoes/10^3/time.txt
printf "" > Experimento/SelecaoAleatoria/execucoes/10^3/time.txt 

for i in {1..30}
do
   echo "Execucao - $i"
   #touch  Experimento/SelecaoAleatoria/execucoes/10^3/logExecucao$i.txt
   ./eg --database="../../problems/p4_10^3.txt"  --cores=$PCORES >> Experimento/SelecaoAleatoria/execucoes/10^3/time.txt

done

printf "1000\t" >> Experimento/SelecaoAleatoria/execucoes/time.txt
./media "Experimento/SelecaoAleatoria/execucoes/10^3/time.txt" >> Experimento/SelecaoAleatoria/execucoes/time.txt
printf "\n" >> Experimento/SelecaoAleatoria/execucoes/time.txt


#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^4 registros"

mkdir Experimento/SelecaoAleatoria/execucoes/10^4
touch Experimento/SelecaoAleatoria/execucoes/10^4/time.txt
printf "" > Experimento/SelecaoAleatoria/execucoes/10^4/time.txt 

for i in {1..30}
do
   echo "Execucao - $i"
   #touch  Experimento/SelecaoAleatoria/execucoes/10^4/logExecucao$i.txt
   ./eg --database="../../problems/p4_10^4.txt"  --cores=$PCORES >> Experimento/SelecaoAleatoria/execucoes/10^4/time.txt

done

printf "10000\t" >> Experimento/SelecaoAleatoria/execucoes/time.txt
./media "Experimento/SelecaoAleatoria/execucoes/10^4/time.txt" >> Experimento/SelecaoAleatoria/execucoes/time.txt
printf "\n" >> Experimento/SelecaoAleatoria/execucoes/time.txt

#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^5 registros"

mkdir Experimento/SelecaoAleatoria/execucoes/10^5
touch Experimento/SelecaoAleatoria/execucoes/10^5/time.txt
printf "" > Experimento/SelecaoAleatoria/execucoes/10^5/time.txt 

for i in {1..30}
do
   echo "Execucao - $i"
   #touch  Experimento/SelecaoAleatoria/execucoes/10^5/logExecucao$i.txt
   ./eg --database="../../problems/p4_10^5.txt"  --cores=$PCORES >> Experimento/SelecaoAleatoria/execucoes/10^5/time.txt

done

printf "100000\t" >> Experimento/SelecaoAleatoria/execucoes/time.txt
./media "Experimento/SelecaoAleatoria/execucoes/10^5/time.txt" >> Experimento/SelecaoAleatoria/execucoes/time.txt
printf "\n" >> Experimento/SelecaoAleatoria/execucoes/time.txt

#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^6 registros"

mkdir Experimento/SelecaoAleatoria/execucoes/10^6
touch Experimento/SelecaoAleatoria/execucoes/10^6/time.txt
printf "" > Experimento/SelecaoAleatoria/execucoes/10^6/time.txt 

for i in {1..30}
do
   echo "Execucao - $i"
   #touch  Experimento/SelecaoAleatoria/execucoes/10^6/logExecucao$i.txt
   ./eg --database="../../problems/p4_10^6.txt"  --cores=$PCORES >> Experimento/SelecaoAleatoria/execucoes/10^6/time.txt

done

printf "1000000\t" >> Experimento/SelecaoAleatoria/execucoes/time.txt
./media "Experimento/SelecaoAleatoria/execucoes/10^6/time.txt" >> Experimento/SelecaoAleatoria/execucoes/time.txt
printf "\n" >> Experimento/SelecaoAleatoria/execucoes/time.txt


#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^7 registros"

mkdir Experimento/SelecaoAleatoria/execucoes/10^7
touch Experimento/SelecaoAleatoria/execucoes/10^7/time.txt
printf "" > Experimento/SelecaoAleatoria/execucoes/10^7/time.txt 

for i in {1..30}
do
   echo "Execucao - $i"
   ./eg --database="../../problems/p4_10^7.txt"  --cores=$PCORES >> Experimento/SelecaoAleatoria/execucoes/10^7/time.txt

done

printf "10000000\t" >> Experimento/SelecaoAleatoria/execucoes/time.txt
./media "Experimento/SelecaoAleatoria/execucoes/10^7/time.txt" >> Experimento/SelecaoAleatoria/execucoes/time.txt
printf "\n" >> Experimento/SelecaoAleatoria/execucoes/time.txt

#Script para plotar o gráfico
#gnuplot scripts_grafico.gnu
#eog Experimento/SelecaoAleatoria/grafico.png



