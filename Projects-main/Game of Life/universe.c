#include "universe.h"

#include <stdlib.h>

struct Universe {
    uint32_t rows;
    uint32_t cols;

    bool **grid;
    bool toroidal;
};

Universe *uv_create(uint32_t rows, uint32_t cols, bool toroidal) {
    Universe *u = (Universe *) calloc(1, sizeof(Universe)); // Allocate space for Universe u
    u->rows = rows;
    u->cols = cols;
    u->toroidal = toroidal;
    bool **matrix = (bool **) calloc(rows, sizeof(bool *)); // Allocating space for rows
    for (uint32_t r = 0; r < rows; r += 1) {
        matrix[r] = (bool *) calloc(cols, sizeof(bool));
    }
    u->grid = matrix;
    return u; //return the universe
}

void uv_delete(Universe *u) {
    uint32_t rows = uv_rows(u);
    for (uint32_t r = 0; r < rows; r++) {
        free(u->grid[r]); //free each column in row
    }
    free(u->grid); //free the row or the grid
    free(u); //then free the universe
}

uint32_t uv_rows(Universe *u) { //gives the rows of the universe
    return u->rows;
}

uint32_t uv_cols(Universe *u) { //gives the columns of the universe
    return u->cols;
}

void uv_live_cell(Universe *u, uint32_t r, uint32_t c) { //resuruct or perserve the life of the cell
    u->grid[r][c] = true;
}

void uv_dead_cell(Universe *u, uint32_t r, uint32_t c) { //kill the living of keep the dead dead
    u->grid[r][c] = false;
}

bool uv_get_cell(Universe *u, uint32_t r, uint32_t c) { //find the living status of a cell
    return u->grid[r][c];
}

bool uv_populate(Universe *u, FILE *infile) { //give life to your universe
    uint32_t r;
    uint32_t c;
    int cell;
    do {
        cell = fscanf(infile, "%d %d\n", &r, &c); //life the cells coordinates in the input
        uv_live_cell(u, r, c);
    } while (cell != EOF);

    return true; //yes I have given life to the universe
}

uint32_t uv_census(Universe *u, uint32_t r, uint32_t c) { //find the number of living
    uint32_t population = 0;
    uint32_t rows = r;
    uint32_t cols = c;

    for (uint32_t r = 0; r < rows; r++) {
        for (uint32_t c = 0; c < cols; c++) {
            if (uv_get_cell(u, r, c)) {
                population++;
            }
        }
    }
    return population;
}

void uv_print(
    Universe *u, FILE *outfile) { //output the final generation somewhere that is not the stdout
    uint32_t rows = uv_rows(u);
    uint32_t cols = uv_cols(u);

    for (uint32_t r = 0; r < rows; r++) {
        for (uint32_t c = 0; c < cols; c++) {
            if (uv_get_cell(u, r, c)) {
                //fprintf(outfile, "o"); //living cells are 'o'
                fputs("o", outfile);
            } else {
                //fprintf(outfile, "."); //dead is shiveled up and '.'
                fputs(".", outfile);
            }
        }
        //fprintf(outfile, "\n");
        fputs("\n", outfile);
    }
}
