
if [ ! -d ./tmp ]; then mkdir ./tmp; fi

if [ -f ./tmp/iem.dat ]; then rm -f ./tmp/iem.dat; fi
if [ -f ./tmp/iec.dat ]; then rm -f ./tmp/iec.dat; fi

if [ -f ./tmp/bem.dat ]; then rm -f ./tmp/bem.dat; fi
if [ -f ./tmp/bec.dat ]; then rm -f ./tmp/bec.dat; fi

if [ -f ./tmp/hem.dat ]; then rm -f ./tmp/hem.dat; fi
if [ -f ./tmp/hec.dat ]; then rm -f ./tmp/hec.dat; fi

if [ -f ./tmp/qem.dat ]; then rm -f ./tmp/qem.dat; fi
if [ -f ./tmp/qec.dat ]; then rm -f ./tmp/qec.dat; fi

for ((r=10;r<1000;r+=10))do
	e=300
	./sorting -i -r $r -p 0 -n $e> ./tmp/i.dat
	./sorting -b -r $r -p 0 -n $e> ./tmp/b.dat
	./sorting -h -r $r -p 0 -n $e> ./tmp/h.dat
	./sorting -q -r $r -p 0 -n $e> ./tmp/q.dat

	ne=$(awk '{print $2}' ./tmp/i.dat)
	
	m=$(awk '{print $5}' ./tmp/i.dat)
	c=$(awk '{print $7}' ./tmp/i.dat)
	echo "$r $m" >> ./tmp/iem.dat #Elements vs Moves
	echo "$r $c" >> ./tmp/iec.dat #Elements vs Compares

	m=$(awk '{print $5}' ./tmp/b.dat)
        c=$(awk '{print $7}' ./tmp/b.dat)
	echo "$r $m" >> ./tmp/bem.dat #Elements vs Moves
        echo "$r $c" >> ./tmp/bec.dat #Elements vs Compares

	m=$(awk '{print $5}' ./tmp/h.dat)
        c=$(awk '{print $7}' ./tmp/h.dat)
        echo "$r $m" >> ./tmp/hem.dat #Elements vs Moves
        echo "$r $c" >> ./tmp/hec.dat #Elements vs Compares

        m=$(awk '{print $5}' ./tmp/q.dat)
        c=$(awk '{print $7}' ./tmp/q.dat)
        echo "$r $m" >> ./tmp/qem.dat #Elements vs Moves
        echo "$r $c" >> ./tmp/qec.dat #Elements vs Compares
	
done

gnuplot <<END

        set terminal pdf
        set output "RvC.pdf"
        set title "Random Seed vs Compares e(300)"
        set xlabel "Random Seed"
        set ylabel "Num Compares"

        plot "./tmp/iec.dat" with linespoints pt 0 ps 1 lc rgb '0x000000' title "Insert",\
        "./tmp/bec.dat" with linespoints pt 0 ps 1 lc rgb '0x0000FF' title "Batcher",\
        "./tmp/hec.dat" with linespoints pt 0 ps 1 lc rgb '0x00FF00' title "Heap",\
        "./tmp/qec.dat" with linespoints pt 0 ps 1 lc rgb '0xFF0000' title "Quick"

	set terminal pdf
        set output "RvM.pdf"
        set title "Random Seed vs Moves e(300)"
        set xlabel "Random Seed"
        set ylabel "Num Moves"

        plot "./tmp/iem.dat" with linespoints pt 0 ps 1 lc rgb '0x000000' title "Insert",\
        "./tmp/bem.dat" with linespoints pt 0 ps 1 lc rgb '0x0000FF' title "Batcher",\
        "./tmp/hem.dat" with linespoints pt 0 ps 1 lc rgb '0x00FF00' title "Heap",\
        "./tmp/qem.dat" with linespoints pt 0 ps 1 lc rgb '0xFF0000' title "Quick"

END

:<<'Graph1'
gnuplot <<END
        set terminal pdf
        set output "EvM.pdf"
        set title "Elements vs Moves r(10)"
        set xlabel "Num Elements"
        set ylabel "Num Moves"

        plot "./tmp/bem.dat" with linespoints pt 0 ps 1 lc rgb '0x0000FF' title "Batcher",\
	"./tmp/qem.dat" with linespoints pt 0 ps 1 lc rgb '0xFF0000' title "Quick"

END

gnuplot <<END

	set terminal pdf
        set output "RvC.pdf"
        set title "Random Seed vs Compares e(300)"
        set xlabel "Random Seed"
        set ylabel "Num Compares"
        
        plot "./tmp/iec.dat" with linespoints pt 0 ps 1 lc rgb '0x000000' title "Insert",\
        "./tmp/bec.dat" with linespoints pt 0 ps 1 lc rgb '0x0000FF' title "Batcher",\
        "./tmp/hec.dat" with linespoints pt 0 ps 1 lc rgb '0x00FF00' title "Heap",\
        "./tmp/qec.dat" with linespoints pt 0 ps 1 lc rgb '0xFF0000' title "Quick"
END
Graph1
	

