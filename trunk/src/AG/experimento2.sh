#GeForce GTX 460 SE

#Contabiliza o número de gerações para se encontrar o melhor indivíduo do problema

echo "Melhor individuo" > logMelhorIndividuo.txt

#KERNEL 0

for i in {1..100}
do
   echo "Execucao - $i"
   ./ag.exe $i >> logMelhorIndividuo.txt
done

cat logMelhorIndividuo.txt

