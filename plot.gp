set key autotitle columnhead

set title "Col=".Col

plot "test.dat" u 1:Col w l

pause -1 "Hit return to continue" 
