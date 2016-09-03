set title "a vs B-field"

set grid

set xtics nomirror
set x2tics 0,50
set y2tics 0,10
set autoscale xfix
set autoscale x2fix
set autoscale yfix
set autoscale y2fix
set xlabel 'B-field'
set ylabel 'a.u.'
set x2label 'Index'
set y2label 'Time [us]'
set yrange [-60:60]
set y2range [0:60]

plot "FitResult.txt" u 2:3 w lp title "a",\
	 "FitResult.txt" u 1:(NaN) axes x2y1 title '',\
	 "FitResult.txt" u 2:4 axes x1y2 w lp title "Ta"
	 
     
     
     
#pause -1 "Hit return to continue" 
