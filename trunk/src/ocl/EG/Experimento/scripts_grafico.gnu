set terminal pdf

set output 'grafico.pdf'	
set title 'Tempo de execução X número de amostras'
set xrange [0:1000000]
set yrange [0:1000]

set xlabel "Número de amostras"
set ylabel "Tempo total de execução (s)"

plot 'Intel Core i3 3240/execucoes/time.txt' u($1):($6) title 'CORE I3 3240' with lines, 'Intel Core i5 3210M/execucoes/time.txt' u($1):($6) title 'CORE I5 3210M' with lines, 'NVIDIA GTX 650 TI/execucoes/time.txt' u($1):($6) title 'Nvidia GTX 650 TI' with lines 


