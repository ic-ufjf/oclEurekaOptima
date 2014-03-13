#GeForce GTX 460 SE

#Contabiliza o número de gerações para se encontrar o melhor indivíduo do problema

echo "Melhor individuo kernel 5" > logMelhorIndividuoKernel5.txt

#KERNEL 0

echo "Kernel 5"
for i in {1..50}
do
   echo "Execucao - $i"
   ./ag.exe -k 5 >> logMelhorIndividuoKernel.txt
    cat logMelhorIndividuoKernel.txt
done


