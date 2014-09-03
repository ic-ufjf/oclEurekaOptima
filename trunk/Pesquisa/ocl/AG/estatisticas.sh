echo "" > Estatisticas.txt

echo "Kernel 0"

#KERNEL 0

echo "Kernel 0"
for i in {1..50}
do
   echo "Execucao - $i"
   ./ag.exe >> logKernel0.txt
done

echo -e "--------------\n"

#KERNEL 1

echo "Kernel 1"

for i in {1..50}
do
    echo "Execucao - $i"
   ./ag.exe -k 1 >> logKernel1.txt
done

echo -e "--------------\n"


#KERNEL 2

echo "Kernel 2"

for i in {1..50}
do
   echo "Execucao - $i"
   ./ag.exe -k 2 >> logKernel2.txt
done

echo -e "--------------\n"

#KERNEL 3

echo "Kernel 3"

for i in {1..50}
do
    echo "Execucao - $i"
   ./ag.exe -k 3 >> logKernel3.txt
done

echo -e "--------------\n"

#KERNEL 4



