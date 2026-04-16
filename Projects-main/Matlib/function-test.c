#include "mathlib.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> //for atoi
#include <unistd.h> //for getopt

//Useage
int main(int argc, char **argv) {
    int opt = 0;
    double x = 0.0;
    while ((opt = getopt(argc, argv, "erscln:")) != -1) {
        switch (opt) {
        case 'e':

            for (double i = -100; i < 100; i += 2) {
                //printf(" %7.4lf % 16.8lf % 16.8lf % 16.15lf\n", i, Exp(i), exp(i), Exp(i) - exp(i));
                printf(" %7.4lf,%16.15lf\n", i, (Exp(i) - exp(i)) / exp(i) * 100);
            }

            break;

        case 'r':
            for (double i = 0; i <= 10000; i += 100) {
                //printf(" %7.4lf % 16.8lf % 16.8lf % 16.15lf\n", i, Sqrt(i), sqrt(i) ,Sqrt(i) - sqrt(i));
                printf(" %7.4lf,%16.15lf\n", i, (Sqrt(i) - sqrt(i)) / sqrt(i) * 100);
            }
            break;

        case 's':
            for (double i = -4 * M_PI; i <= 4 * M_PI; i += 0.1) {
                //printf(" %7.4lf % 16.8lf % 16.8lf % 16.15lf\n", i, Sin(i), sin(i), Sin(i) - sin(i));
                printf(" %7.4lf,%16.15lf\n", i, (Sin(i) - sin(i)));
                //printf(" %7.4lf,%16.15lf\n", i, (Sin(i) - sin(i))/sin(i) * 100);
            }

            break;
        case 'c':
            for (double i = -2 * M_PI; i <= 2 * M_PI; i += 0.1) {
                //printf(" %7.4lf % 16.8lf % 16.8lf % 16.15lf\n", i, Cos(i), cos(i), Cos(i) - cos(i));
                printf(" %7.4lf,%16.15lf\n", i, Cos(i) - cos(i));
                //printf(" %7.4lf,%16.15lf\n", i, (Cos(i) - cos(i))/cos(i));
            }

            break;

        case 'l':

            for (double i = 1; i <= 100; i += 0.5) {
                //printf(" %7.4lf % 16.8lf % 16.8lf % 16.15lf\n", i, Log(i), log(i), Log(i) - log(i));
                //printf(" %7.4lf,%16.15lf\n", i, Log(i) - log(i));
                printf(" %7.4lf,%16.15lf\n", i, (Log(i) - log(i)) / log(i));
            }

            break;
        case 'n': x = strtod(optarg, NULL);

        default: printf("Nothing\n"); return 1;
        }
    }
    return 0;
}
