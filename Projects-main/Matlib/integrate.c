#include "functions.h"
#include "mathlib.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> //for atoi
#include <unistd.h> //for getopt

//Useage
int main(int argc, char **argv) {
    int opt = 0;
    int partitions = 100; //Default 100 partitions
    double min_x = 0.0;
    double max_x = 10;
    bool a_f = false;
    bool b_f = false;
    bool c_f = false;
    bool d_f = false;
    bool e_f = false;
    bool f_f = false;
    bool g_f = false;
    bool h_f = false;
    bool i_f = false;
    bool j_f = false;
    while ((opt = getopt(argc, argv, "tabcdefghijp:q:n:H")) != -1) {
        switch (opt) {
        case 't': printf("Sqrt(x) = %f\n", integrate(Sqrt, min_x, max_x, partitions)); break;

        case 'a':
            min_x = 0.0;
            max_x = 1.0;
            a_f = true;
            /*
			if (Abs(min_x) > 1 || Abs(max_x) > 1) {
				printf("Range: [-1,1]\n");
				break;
			}
			printf("Sqrt(1 - x^4) = %f\n", integrate(a,min_x,max_x, partitions));
			*/
            break;

        case 'b':
            /*
			if (min_x < 0) {
				printf("Range: (0,1) and (1,inf)\n");
				break;
			}
			*/
            b_f = true;
            min_x = 2.0;
            max_x = 3.0;
            //printf("1/log(x) = %f\n", integrate(b, min_x, max_x, partitions));
            break;

        case 'c':
            min_x = -10.0;
            max_x = 10.0;
            c_f = true;
            //printf("e^(-x^2) = %f\n", integrate(c, min_x, max_x, partitions));
            break;

        case 'd':
            min_x = -M_PI;
            max_x = M_PI;
            d_f = true;
            //printf("sin(x^2) = %f\n", integrate(d, min_x, max_x, partitions));
            break;

        case 'e':
            min_x = -M_PI;
            max_x = M_PI;
            e_f = true;
            //printf("cos(x^2) = %f\n", integrate(e, min_x, max_x, partitions));
            break;

        case 'f':
            min_x = 2.0;
            max_x = 10.0;
            f_f = true;
            //printf("log(log(x)) = %f\n", integrate(f, min_x, max_x, partitions));
            break;

        case 'g':
            min_x = -4 * M_PI;
            max_x = 4 * M_PI;
            g_f = true;
            //printf("sin(x)/x = %f\n", integrate(g, min_x, max_x, partitions));
            break;

        case 'h':
            min_x = 1.0;
            max_x = 10.0;
            h_f = true;
            //printf("(e^-x)/x = %f\n", integrate(h, min_x, max_x, partitions));
            break;

        case 'i':
            min_x = 0.0;
            max_x = 1.0;
            i_f = true;
            //printf("e^e^x = %f\n", integrate(i, min_x, max_x, partitions));
            break;

        case 'j':
            min_x = 0;
            max_x = M_PI;
            j_f = true;
            //printf("sqrt(sin(x)^2 + cos(x)^2) = %f\n", integrate(j, min_x, max_x, partitions));
            break;

        case 'p': min_x = strtod(optarg, NULL); break;

        case 'q': max_x = strtod(optarg, NULL); break;

        case 'n':
            partitions = atoi(optarg); //do not use atoi, change later
            //printf("partitions \n");
            break;

        case 'H':
            printf("Usage:");
            printf("-[?](function) -p (low) -q (double high) -n (int partitions)\n");
            printf("functions: [a,b,c,d,e,f,g,h,i,j]\n");
            printf("a) sqrt(1-x^4)\n");
            printf("b) 1/log(x)\n");
            printf("c) e^-(x^2)\n");
            printf("d) sin(x^2)\n");
            printf("e) cos(x^2)\n");
            printf("f) log(log(x))\n");
            printf("h) (e^-x)/x\n");
            printf("i) e^e^x\n");
            printf("j) sqrt(sin(x)^2 + cos(x)^2)\n");

        default: printf("Nothing\n"); return 1;
        }
    }

    if (a_f) {
        printf("sqrt(1-x^4) ,%f ,%f, %i, %.15f\n", min_x, max_x, partitions,
            integrate(a, min_x, max_x, partitions));
    }
    if (b_f) {
        printf("1/log(x) ,%f ,%f, %i, %.15f\n", min_x, max_x, partitions,
            integrate(b, min_x, max_x, partitions));
    }
    if (c_f) {
        printf("e^-(x^2) ,%f ,%f, %i, %.15f\n", min_x, max_x, partitions,
            integrate(c, min_x, max_x, partitions));
    }
    if (d_f) {
        printf("sin(x^2) ,%f ,%f, %i, %.15f\n", min_x, max_x, partitions,
            integrate(d, min_x, max_x, partitions));
    }
    if (e_f) {
        printf("cos(x^2) ,%f ,%f, %i, %.15f\n", min_x, max_x, partitions,
            integrate(e, min_x, max_x, partitions));
    }
    if (f_f) {
        printf("log(log(x)) ,%f ,%f, %i, %.15f\n", min_x, max_x, partitions,
            integrate(f, min_x, max_x, partitions));
    }
    if (g_f) {
        printf("sin(x)/x ,%f ,%f, %i, %.15f\n", min_x, max_x, partitions,
            integrate(g, min_x, max_x, partitions));
    }
    if (h_f) {
        printf("(e^-x)/x ,%f ,%f, %i, %.15f\n", min_x, max_x, partitions,
            integrate(h, min_x, max_x, partitions));
    }
    if (i_f) {
        printf("e^e^x ,%f ,%f, %i, %.15f\n", min_x, max_x, partitions,
            integrate(i, min_x, max_x, partitions));
    }
    if (j_f) {
        printf("sqrt(sin(x)^2 + cos(x)^2) ,%f ,%f, %i, %.15f\n", min_x, max_x, partitions,
            integrate(j, min_x, max_x, partitions));
    }

    return 0;
}
