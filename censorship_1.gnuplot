set term png size 600, 400
set output 'censorship_1.png'
title = "Versión Secuencial"
plot 'censorship_1.dat' using 1:3:2 title title with filledcurves
