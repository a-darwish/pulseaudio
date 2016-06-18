set terminal pngcairo enhanced size 640,480 font 'Verdana,10'
set output "audio.png"
set yrange [-1:1]
set ytics -1,0.1,1		# 0.1 jumps
set key outside

# FIXME: Revisit channel names.. we're not sure they're of
# the same order in the file
plot "audio.plot" using 2 with lines title 'front-left', '' using 3 with lines title 'front-right', '' using 4 with lines title 'rear-left', '' using 5 with lines title 'rear-right', '' using 6 with lines title 'front-center', '' using 7 with lines title 'lfe'
