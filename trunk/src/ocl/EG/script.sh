echo "" > logExecucao.txt

for i in {1..100}
do
   echo "Execucao - $i"
   echo "" > execucoes/logExecucao$i.txt
   ./ag.exe --database="problems/teste.txt"  >>  execucoes/logExecucao$i.txt
done
