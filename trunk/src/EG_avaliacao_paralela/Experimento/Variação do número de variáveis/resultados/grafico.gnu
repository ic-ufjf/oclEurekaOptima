set xrange[1:15]
set yrange[0:120]
set terminal eps
set pointsize 0.8

set xlabel "Número de variáveis"
set ylabel "Tempo de execução (s)"

set key left

set output 'exp2.1.eps'
plot 'time_comp2.dat' using 1:3 title 'Compilação' with linespoints lw 2, 'time_int2.dat' using 1:3 title 'Interpretação' with linespoints lw 2

set ylabel "Tempo de avaliação (s)"

set output 'avaliacao2.1.eps'
plot 'time_comp.dat' using 1:2 title 'Compilação' with linespoints lw 2, 'time_int2.dat' using 1:2 title 'Interpretação' with linespoints lw 2


