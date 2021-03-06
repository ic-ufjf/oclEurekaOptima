set terminal eps

set output 'speedup.eps'	
set xrange [0:1000000]
set yrange [0:70]

set key left

set pointsize 0.4


set xlabel "number of data points"
set ylabel "speedup"

plot 'Intel Core i3 3240 (AMD)/execucoes/time.txt' u($1):($7) title 'CORE I3 3240' with linespoints lw 2,  'Intel Core i5 3210M (4 cores - AMD)/execucoes/time.txt' u($1):($7) title 'CORE I5 3210M' with linespoints lw 2,'NVIDIA GTX 650 TI/execucoes/time.txt' u($1):($7) title 'Nvidia GTX 650 TI' with linespoints lw 2
