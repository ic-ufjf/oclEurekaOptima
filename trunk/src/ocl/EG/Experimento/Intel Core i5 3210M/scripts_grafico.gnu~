set terminal png

set output 'grafico.png'	
set title 'Tempo de execução X número de amostras'
set xrange [0:100000]
set yrange [0:100]

set xlabel "Número de amostras"
set ylabel "Tempo de execução (s)"

plot 'execucoes/time.txt' u($1):($2) title 'Core i3' with lines


