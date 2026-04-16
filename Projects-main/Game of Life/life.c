#include "universe.h"
#include <curses.h>
#include <ncurses.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#define DELAY 50000

void cell_survival(Universe *u, Universe *uo, bool toroidal, uint32_t r,
    uint32_t c); //Determines if the cell survives to next generation

void uv_next_gen(
    Universe *u, Universe *uo, bool toroidal); //Creates the next generation in alternate universe

void curs_screen(Universe *u, bool curse); //Animates 1 generation

int main(int argc, char **argv) {
    FILE *infile;
    FILE *outfile;
    char *infile_name;
    char *outfile_name;
    bool put_out_file = false;
    bool toroidal = false; //default is to be a flat earther
    bool curse = true; //default is to have ncurses
    uint32_t generations = 100;
    int ch;
    while ((ch = getopt(argc, argv, "tsn:i:o:")) != -1) {
        switch (ch) {
        case 't':
            toroidal = true; //become a hollow earther
            break;
        case 's': curse = false; break; //do not watch a animation about life
        case 'n':
            if (strtol(optarg, NULL, 10) < 0) { //Checking if number of generations 0 or more
                fprintf(stderr, "Invalid number of Generations\n");
                return 0;
            }
            generations = (uint32_t) strtol(
                optarg, NULL, 10); //numeber of generations wanted to be seen in base 10
            break;
        case 'i':
            infile_name = optarg; //which universe do you want to see?
            break;
        case 'o':
            put_out_file = true;
            outfile_name = optarg; //put the last generation not in stdout
            break;
        default: break;
        }
    }

    if (infile_name == NULL) {
        fprintf(stderr, "No file input. Use option -i [filename] \n");
        return 0;
    }

    infile = fopen(infile_name, "r"); //read the inputted file

    uint32_t rows, cols;
    fscanf(infile, "%d %d\n", &rows, &cols); //first line contains dimensions of the universe

    if (rows <= 0 || cols <= 0) {
        fprintf(stderr, "Invalid Rows or Columns, check top line of file.\n");
        return 0;
    }

    Universe *u = uv_create(rows, cols, toroidal);
    Universe *uo = uv_create(rows, cols, toroidal); //other universe

    uint32_t gen_count = 0;
    uv_populate(u, infile); //populating in the 0th generation
    fclose(infile);
    if (curse)
        initscr();
    for (gen_count = 0; gen_count <= generations; gen_count++) {
        if (gen_count == 0) {
            curs_screen(u, curse);
        }
        if (gen_count % 2 == 1) { //odd generations go from u to uo
            curs_screen(uo, curse);
            uv_next_gen(u, uo,
                toroidal); //current generation is in universe u put the next generation in universe uo
        }

        if (gen_count % 2 == 0 && gen_count != 0) { //even generations go from uo to u
            curs_screen(u, curse);
            uv_next_gen(uo, u,
                toroidal); //current generation is in universe uo put the next generation in universe u
        }
    }

    endwin();
    //printf("\n");
    if (put_out_file) {
        outfile = fopen(outfile_name, "w");
        //outfile = fopen(outfile_name, "a");
    } else {
        outfile = stdout;
    }
    if (generations % 2 == 0)
        uv_print(u, outfile); //if the last generation is even it is stored in u
    if (generations % 2 == 1)
        uv_print(uo, outfile); //if the last generation is odd it is stored in uo
    if (put_out_file)
        fclose(outfile);

    uv_delete(u);
    uv_delete(uo);

    return 0;
}

void uv_next_gen(
    Universe *u, Universe *uo, bool toroidal) { //generate the next generation in parallel universe

    uint32_t rows = uv_rows(u);
    uint32_t cols = uv_cols(u);

    for (uint32_t r = 0; r < rows; r++) { //loop through universe
        for (uint32_t c = 0; c < cols; c++) {
            cell_survival(u, uo, toroidal, r, c); //check if each cell will surive of resuruct
        }
    }
}

void cell_survival(Universe *u, Universe *uo, bool toroidal, uint32_t r, uint32_t c) {
    uint32_t living_neighbors = 0; //counting the neighbors
    uint32_t max_row = uv_rows(u);
    uint32_t max_col = uv_cols(u);
    int32_t rr = (int32_t) r;
    int32_t cc = (int32_t) c;
    for (int32_t nr = -1; nr <= 1; nr++) { //3 neigbor rows above, current, below
        for (int32_t nc = -1; nc <= 1; nc++) { //3 neigbor cols left, curent, right

            bool r_valid = (r + nr >= 0 && r + nr < max_row); //is it within the flat earther bounds
            bool c_valid = (c + nc >= 0 && c + nc < max_col);

            if (r_valid
                && c_valid) { //if flat earther approved then if alive incrememnt living neighbors
                if (r_valid)
                    rr = r + nr;
                if (c_valid)
                    cc = c + nc;
                if (uv_get_cell(u, rr, cc))
                    living_neighbors++;

            } else if (
                toroidal) { //if a hollow earther and does not flat earth approved then loop around
                rr = (r + nr) % max_row;
                cc = (c + nc) % max_col;

                if (rr < 0)
                    rr = max_row + rr;
                if (cc < 0)
                    cc = max_col + cc;
                if (uv_get_cell(u, rr, cc))
                    living_neighbors++;
            }
        }
    }

    if (uv_get_cell(u, r, c))
        living_neighbors--; // you are not your own neigbor
    //3 neighbors means survive or stop being dead
    //2 neighbors means surive
    //other means dead
    if (living_neighbors == 3 || (uv_get_cell(u, r, c) && living_neighbors == 2)) {
        uv_live_cell(uo, r, c);
    } else {
        uv_dead_cell(uo, r, c);
    }
}

void curs_screen(Universe *u, bool curse) {
    if (!curse) {
        return;
    }
    uint32_t rows = uv_rows(u);
    uint32_t cols = uv_cols(u);
    for (uint32_t r = 0; r < rows; r++) {
        for (uint32_t c = 0; c < cols; c++) {
            if (uv_get_cell(u, r, c))
                mvprintw(r, c, "o");
            else
                mvprintw(r, c, " ");
        }
    }
    refresh();
    usleep(DELAY);
    clear();
}
