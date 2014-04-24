 set terminal pdf

set output 'grafico.pdf'	
set title 'Tempo de execução X número de amostras'
set xrange [0:100000]
set yrange [0:100]

set xlabel "Número de amostras"
set ylabel "Tempo de execução (s)"

plot 'Intel Core i3 3240/execucoes/time.txt' u($1):($2) title 'CORE I3 3240' with lines, 'Intel Core i5 3210M/execucoes/time.txt' u($1):($2) title 'CORE I5 3210M' with lines, 'NVIDIA GTX 650 TI/execucoes/time.txt' u($1):($2) title 'Nvidia GTX 650 TI' with lines 


