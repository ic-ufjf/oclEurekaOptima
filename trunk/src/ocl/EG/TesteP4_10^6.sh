
echo "Execução ge - P4(x) com 10^6 registros"

mkdir Experimento
mkdir Experimento/execucoes

mkdir Experimento/execucoes/10^6
touch Experimento/execucoes/10^6/time.txt
printf "" > Experimento/execucoes/10^6/time.txt 

for i in {1..30}
do
   echo "Execucao - $i"
   touch  Experimento/execucoes/10^6/logExecucao$i.txt
   ./eg --database="problems/p4_10^6.txt" >> Experimento/execucoes/10^6/time.txt

done

printf "1000000\t" >> Experimento/execucoes/time.txt
./media "Experimento/execucoes/10^6/time.txt" >> Experimento/execucoes/time.txt
printf "\n" >> Experimento/execucoes/time.txt

