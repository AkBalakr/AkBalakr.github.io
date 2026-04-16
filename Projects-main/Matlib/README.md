# Assignment 2: Numerical Integration

## Description:
	This program contains a remake of part of the math.h library.
	The math.h library has the functions:
		double Exp(double x)  #returns e^x
		double Sin(double x)  #returns sin(x)
		double Cos(double x)  #returns cos(x)
		double Sqrt(double x) #returns Sqrt(x) Do not make x < 0
		double Log(double x)  #returns Ln(x)
		double integrate(double (*f)(double), double a, double b, uint32_t n) #returns integration of 
		a given function in range [a,b] using n partitions.

	integrate.c integrates 1 to 10 functions from function.h

## Build:
	make all

## Run:
	./integrate -function(a to j) -n(number of partions) -p(min of range x) -q(max of range x) 
		Note: if -n -p -q are not added they each have their own default ranges
## Clean:
	make clean
