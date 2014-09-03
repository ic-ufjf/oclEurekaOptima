set xrange[10:10000000]
set yrange[0:300]
set terminal eps
set pointsize 0.8

set xlabel "Número de registros"
set ylabel "Tempo de execução (s)"

set key left

set output 'exp1.eps'
plot 'time_comp.dat' using 1:3 title 'Compilação' with linespoints lw 2, 'time_int.dat' using 1:3 title 'Interpretação' with linespoints lw 2

set ylabel "Tempo de avaliação (s)"

set output 'avaliacao1.eps'
plot 'time_comp.dat' using 1:2 title 'Compilação' with linespoints lw 2, 'time_int.dat' using 1:2 title 'Interpretação' with linespoints lw 2




