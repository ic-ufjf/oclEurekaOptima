set terminal pdf

set output 'grafico.pdf'	
set title 'Número de amostras X Tempo de execução'
set xrange [0:1000000]
set yrange [0:1700]

set pointsize 0.4

set xlabel "Número de amostras"
set ylabel "Tempo total de execução (s)"

plot 'Intel Core i3 3240 (AMD)/execucoes/time.txt' u($1):($6) title 'CORE I3 3240' with linespoints ,  'Intel Core i5 3210M (1 core - AMD)/execucoes/time.txt' u($1):($6) title 'CORE I5 3210M (1 core)' with linespoints, 'Intel Core i5 3210M (4 cores - AMD)/execucoes/time.txt' u($1):($6) title 'CORE I5 3210M (4 cores)' with linespoints,'NVIDIA GTX 650 TI/execucoes/time.txt' u($1):($6) title 'Nvidia GTX 650 TI' with linespoints
