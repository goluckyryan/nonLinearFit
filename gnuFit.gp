set key autotitle columnhead

fitFlag = 5;

yIndex = 240
startX = 250
a = 20
Ta = 20
b = -10
Tb = 80
FileName = "test.dat"


col = yIndex + 2
set grid

stats FileName u 1:col nooutput
set xrange [-100:400]
set yrange [STATS_min_y*1.1:STATS_max_y*1.1]

set fit logfile "gnufit.log"
!del gnufit.log

set fit maxiter 300
set fit limit 1e-6


#===================================== fit for 4
if( fitFlag == 4){

f(x) = a*exp(-x/Ta) + b*exp(-x/Tb)

fit f(x) FileName u 1:col every ::(startX) via a, Ta, b, Tb

set title sprintf("Col = %d, (a, Ta, b, Tb) = (%4.2f, %4.2f, %4.2f, %4.2f) \n error = (%4.2f, %4.2f, %4.2f, %4.2f) \n X(%d, end)", yIndex, a, Ta, b, Tb, a_err, Ta_err, b_err, Tb_err, startX)

plot "test.dat" u 1:col every ::2 w l,\
	 "test.dat" u 1:col every ::(startX+2) w l  title "fit data" lt rgb "#FF00FF",\
      a * exp(-x/Ta)   title "a * exp(-x/Ta)" lt rgb "#0000FF",\
      b * exp(-x/Tb) title "b * exp(-x/Tb)" lt rgb "#33FF33",\
      a * exp(-x/Ta)+b * exp(-x/Tb)  title "a * exp(-x/Ta)+b*exp(-x/Tb)" lt rgb "#000000"
}

if( fitFlag == 5){

f(x) = a*exp(-x/Ta) + b*exp(-x/Tb) + c

fit f(x) FileName u 1:col every ::(startX) via a, Ta, b, Tb, c

set title sprintf("Col = %d, (a, Ta, b, Tb, c) = (%4.2f, %4.2f, %4.2f, %4.2f, %4.2f) \n error = (%4.2f, %4.2f, %4.2f, %4.2f, %4.2f) \nX(%d,end)", yIndex, a, Ta, b, Tb, c, a_err, Ta_err, b_err, Tb_err, c_err, startX)


plot "test.dat" u 1:col every ::2 w l,\
	 "test.dat" u 1:col every ::(startX+2) w l  title "fit data" lt rgb "#FF00FF",\
      a * exp(-x/Ta)   title "a * exp(-x/Ta)" lt rgb "#0000FF",\
      b * exp(-x/Tb) title "b * exp(-x/Tb)" lt rgb "#33FF33",\
      a * exp(-x/Ta)+b * exp(-x/Tb) +c title "a * exp(-x/Ta)+b*exp(-x/Tb) + c" lt rgb "#000000"
}

#pause -1 "Hit return to continue" 
