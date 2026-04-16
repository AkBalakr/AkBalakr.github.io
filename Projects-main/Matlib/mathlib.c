#include "mathlib.h" // copy pastes the function prototypes

#include <math.h>
#include <stdbool.h>
#include <stdint.h> // allows for uint32_t
#include <stdio.h>

double Fmod(double x, double y) { //returns x%y
    while (Abs(x) > y) {
        if (x < 0) {
            x += y;
        }
        if (x > 0) {
            x -= y;
        }
    }
    return x;
}

double Exp(double x) {
    bool neg = x < 0;
    if (neg) {
        x = -x;
    }

    // e^x = summation k[0, inf) x^k/k!
    // pattern is previous * (x/k)

    double final_answer = 0.0; // Summation of previous
    double increment = 0.0; // increment = previous * (x/k);
    double previous = 0.0; // remembers the last increment

    int k = 0; // loop/summation term counter

    // Probably make into do_while
    if (k == 0) { // When k is 0 increment is 1
        increment = 1.0; // x^0/0! = 1
        previous = increment;
        final_answer += increment;
        k++;
    }

    while (EPSILON < increment) { // if the incremenet is less than epsilon
        // increment
        increment = previous * (x / k);
        previous = increment; // remembering the last increment
        //printf("increment: %f\n", increment);
        final_answer += increment;

        k++; // adding to loop count
    }
    if (neg) {
        return 1 / final_answer;
    }
    return final_answer; //return final summation
}

double Sin(double x) {
    x = Fmod(x, 2 * M_PI);
    double final_answer = 0.0; // Summation of previous
    double increment = 0.0;
    double previous = 0.0; // remembers the last increment

    int k = 1; // k starts at 1 for sin()

    if (k == 1) { // When k is 0 increment is 1
        increment = x;
        previous = increment;
        // printf("increment: %f\n", increment);
        final_answer += increment;
        k += 2;
    }

    while (Abs(increment) > EPSILON) {
        increment = (-1) * previous * (x * x) / (k * (k - 1));
        previous = increment;
        final_answer += increment;
        // printf("increment: %f\n", increment);
        k += 2;
    }

    return final_answer;
}

double Cos(double x) {
    x = Fmod(x, 2 * M_PI);
    double final_answer = 0.0; // Summation of previous
    double increment = 0.0;
    double previous = 0.0; // remembers the last increment

    int k = 0; // k starts at 1 for sin()

    if (k == 0) { // When k is 0 increment is 1
        increment = 1;
        previous = increment;
        // printf("increment: %f\n", increment);
        final_answer += increment;
        k += 2;
    }

    while (Abs(increment) > EPSILON) {
        increment = (-1) * previous * (x * x) / (k * (k - 1));
        previous = increment;
        final_answer += increment;
        // printf("increment: %f\n", increment);
        k += 2;
    }

    return final_answer;
}

//From Assignment 2 Document
double Sqrt(double x) {
    double old_guess = 0.0; // Old guess
    double new_guess = 1.0; // New guess
    double f = 1.0;
    while (x > 1) {
        x /= 4.0;
        f *= 2.0;
    }
    while (Abs(new_guess - old_guess)
           > EPSILON) { // If the difference between old and new is small then stop
        old_guess = new_guess; // Remembering old value
        new_guess = 0.5 * (new_guess + x / new_guess); // Making a new guess
    }
    return f * new_guess;
}

//From Assignment 2 Document
double Log(double x) {
    double guess = 1;
    double g_val = Exp(guess);
    double f = 0.0;
    double e = Exp(1);
    if (Abs(x) < EPSILON) { // Ln(0) = -inf
        return -INFINITY;
    }
    while (x > e) {
        x /= e;
        f += 1.0;
    }

    while (Abs(g_val - x) > EPSILON) {
        guess = guess + ((x / g_val) - 1);
        //printf("guess: %f\n", guess);
        //printf("%.15lf\n",(g_val - x));
        g_val = Exp(guess);
    }

    return f + guess;
}

double integrate(double (*f)(double), double a, double b, uint32_t n) {
    double min_x = a;
    double max_x = b;
    double part = n; //Number of Partitions
    double del_x = (max_x - min_x) / part; //delta x
    double answer = 0.0;
    double increment = 0.0;
    double f_xk = 0.0; //Function at value xk: F(xk)
    uint32_t k = 0;

    //printf("Min: %f	Max: %f\n", min_x, max_x);

    if (k == 0) {
        increment = (*f)(min_x + k * del_x) * (del_x / 3);
        answer += increment;
        printf("%f,%f\n", min_x + k * del_x, answer);
        k++;
    }
    while (k <= n) {
        f_xk = (*f)(min_x + k * del_x) * (del_x / 3);
        if (k == n || f_xk < EPSILON) {
            increment = f_xk;
        } else if (k % 2 == 0) {
            increment = 2 * f_xk;
        } else if (k % 2 == 1) {
            increment = 4 * f_xk;
        }
        answer += increment;
        printf("%f,%f\n", min_x + k * del_x, answer);
        //printf("Increment: %f :x = %f\n" ,increment , min_x + k * del_x );
        k++;
    }
    return (answer);
}

//https:www.youtube.com/watch?v=BRsv3ZXoHto
