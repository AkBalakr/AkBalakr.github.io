#!/usr/bin/env bash

# Example data is sent to /tmp/output.
#cat > ./tmp/output <<herestr
#sqrt(1 - x^4),0.000000,1.000000,100
#herestr
# Get the title from the output.
title=$(head -n 1 ./tmp/output | awk '{print $1}')

# Put all but first line of output in data file.
#tail -n +2 i
echo "$(awk '{print $1, $2}' ./tmp/output | tail -n +2)" > ./tmp/data
#echo "$(awk '{print $1, $2}' ./tmp/output)" > ./tmp/data
#echo "$(awk '{print $1, $2}' ./tmp/output | sed '$d')" > ./tmp/data
# Plot the data, specifying that commas are used as the delimiter.
gnuplot <<EOF
    set terminal png
    set output "example.png"
    set datafile separator ","
    set xlabel "x"
    set ylabel "y"
    set title "$title"
    set zeroaxis
    plot "./tmp/data" with linespoints pt 0 title ""
EOF
