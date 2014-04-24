make -B
make media -B


#--------------------------------------------------------

echo "Execução ge - P4(x) com 10^3 registros"

mkdir Experimento/execucoes/10^3
touch Experimento/execucoes/10^3/time.txt
printf "" > Experimento/execucoes/10^3/time.txt 

for i in {1..50}
do
   echo "Execucao - $i"
   touch  Experimento/execucoes/10^3/logExecucao$i.txt
   ./eg --database="problems/p4_10^3.txt"  >> Experimento/execucoes/10^3/time.txt

done



