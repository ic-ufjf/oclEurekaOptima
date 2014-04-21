set terminal png

set output 'Experimento/grafico.png'	
set title 'Tempo de execução X número de amostras'
set xrange [0:100000]
set yrange [0:4]

set xlabel "Numero de amostras (R)"
set ylabel "Tempo de execução (P)"

plot 'execucoes/time.txt' u($1):($2) title 'Tempo'


