#!/bin/bash

make clean && make collatz	 # Rebuild the sincos executable.
if [ ! -d ./tmp ]; 
then 
	mkdir ./tmp
fi

if [ ! -f ./tmp/lengths.dat ]  && [ ! -f ./tmp/maximum.dat ];
then
	for x in {2..10000};do
		./collatz -n $x > ./tmp/c.dat
		#sleep 1
		l=$(wc -l ./tmp/c.dat)
	      	m=$(sort -nr ./tmp/c.dat|head -1)
	
		echo "$x $l" >> ./tmp/lengths.dat        # Place the data points into a file.
	        echo "$x $m" >> ./tmp/maximum.dat
	done
fi


max=$(awk "{print $2}" ./tmp/lengths.dat |sort -nr |head -1)

echo "$(awk '{print $2}' ./tmp/lengths.dat | sort -n | uniq -c)" >> ./tmp/hist.dat
echo "$(awk '{print $2, $1}' ./tmp/hist.dat)" >> ./tmp/histogram.dat

#cat ./tmp/maximum.dat
#cat ./tmp/collatz.dat
#cat ./tmp/histogram.dat

# This is the heredoc that is sent to gnuplot.
gnuplot <<END

	
	set terminal pdf

	set output "sequence_lengths.pdf"
        set title "Collatz Sequence Lengths"
        set xlabel "n"
        set ylabel "length"
        set zeroaxis
	set yrange [0:300]
        plot "./tmp/lengths.dat" pt 0 ps 1 title ""

	set output "maximum.pdf"
    	set title "Maximum Collatz Sequence Value"
    	set xlabel "n"
    	set ylabel "value"
    	set zeroaxis
	set yrange [0:100000]
	plot "./tmp/maximum.dat" pt 0 ps 1 title ""

	set output "histogram.pdf"
	#set style data histogram
	#set style histogram clustered
        set title "Collatz Sequence Lengths - Histogram"
        set xlabel "length"
        set ylabel "frequency"
	set boxwidth 0.5
        set zeroaxis
        set yrange [0:200]
	set xrange [0:225]
	plot "./tmp/histogram.dat" using 2:xticlabels(100) with histogram title ""
        #plot "./tmp/histogram.dat" pt 0 title ""

END

