 set terminal pdf

set output 'grafico.pdf'	
set title 'Tempo de execução X número de amostras'
set xrange [0:1000000]
set yrange [0:100]

set style line 1 lt 1 lw 2 pt 2 pi 0 ps 0.5

set xlabel "Número de amostras"
set ylabel "Tempo de execução (s)"
 
plot 'execucoes/time.txt' u($1):($2) title 'NVIDIA GTX 650 TI' with linespoints  ls 1


