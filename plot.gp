set key autotitle columnhead

set title sprintf("Col = %d, (a, Ta, b, Tb) = (%4.2f, %4.2f, %4.2f, %4.2f) \n X(%d, %d)", Col, a, Ta, b, Tb, startX, endX)

set grid

set xrange [-40:160]
set yrange [-abs(a-b):abs(a-b)]

plot "test.dat" u 1:Col w l,\
	 "test.dat" u 1:Col every ::startX w l  title "fit data" lt rgb "#FF00FF",\
      a * exp(-x/Ta)   title "a * exp(-x/Ta)" lt rgb "#0000FF",\
      -b * exp(-x/Tb) title "b * exp(-x/Tb)" lt rgb "#33FF33",\
      a * exp(-x/Ta)-b * exp(-x/Tb) title "a * exp(-x/Ta)-b*exp(-x/Tb)" lt rgb "#000000"

pause -1 "Hit return to continue" 
