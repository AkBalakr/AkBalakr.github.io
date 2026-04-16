#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include "huffman.h"
#include "defines.h"
#include "code.h"
#include "io.h"
#include "header.h"

void usage(void) {
    fprintf(stderr, "usage: encrypt [-hiov]\n");
    fprintf(stderr,
        "-h : Prints out a help message describing the purpose of the program \t\n -i:Specifies "
        "the input file to encode using Huffman coding.t\n-o outfile : Specifies the output file "
        "to write the compressed input to. \t\n -v : Prints compression statistics.\t\n");
    exit(EXIT_FAILURE);
}

float get_spacesaving(int inputsize, int outsize) {

    return 100 * (1 - ((float) outsize / (float) inputsize));
}
mode_t getfilepermission(char *filename) {

    struct stat sb;
    mode_t file_permision;

    if (stat(filename, &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    file_permision = sb.st_mode;
    //printf("File permission: %o (octal)\n", (unsigned int) file_permision);
    return (unsigned int) file_permision;
}

int get_filesize(char *filename) {
    int fd = open(filename, O_RDONLY, S_IRUSR | S_IRGRP);
    struct stat statbuf;
    if (fd == -1) {
        printf("failed to open %s\n", filename);
        exit(EXIT_FAILURE);
    }
    if (fstat(fd, &statbuf) == -1) {
        printf("failed to fstat \n");
        exit(EXIT_FAILURE);
    }
    //int status = fstat(fd, &statbuf);
    //printf(" >>>> %d", status);
    close(fd);
    return statbuf.st_size;
}

//Find the tree size
int size(Node *node) {
    if (node == NULL)
        return 0;
    else
        return (size(node->left) + 1 + size(node->right));
}

void postorder_print(Node *root) {

    if (root != NULL) {

        postorder_print(root->left);
        postorder_print(root->right);

        printf(" %c->f(%" PRId64 ") \n", root->symbol, root->frequency);
    }
}

void inorder_print(Node *root) {

    if (root != NULL) {
        inorder_print(root->left);
        printf("%c->f(%" PRId64 ") \n", root->symbol, root->frequency);
        inorder_print(root->right);
    }
}

int main(int argc, char **argv) {
    FILE *input_fp = stdin;
    //FILE *output_fp = stdout;
    char *input_filename = "files/input.txt";
    char *output_filename = "files/encrypt_out.txt";
    bool input_file = false;
    bool output_file = false;
    bool verbose = false;
    Header header;
    char ch;

    while ((ch = getopt(argc, argv, "i:o:vh")) != -1) {
        switch (ch) {
        case 'h': //displays program synopsis and usage.
            usage();
            break;
        case 'i': //Specifies the input file to encode using Huffman coding.
            input_filename = optarg;
            input_file = true;
            break;
        case 'o':
            output_filename = optarg; //Specifies the output file to write the compressed input to.
            output_file = true;
            break;
        case 'v': //Prints compression statistics
            verbose = true;
            break;
        default: break;
        }
    }

    header.permissions = 0600;
    //printf("Default header.permissions  %" PRIu16 "\n", header.permissions);
    if (input_file) {
        if (!(input_fp = fopen(input_filename, "r"))) {
            perror(input_filename);
            return 1;
        }
        header.permissions = getfilepermission(input_filename);
        //printf("Override header.permissions  %" PRIu16 "\n", header.permissions);
    }

    //printf("Start .....\n");
    uint64_t extent_array[ALPHABET];

    for (int i = 0; i < ALPHABET; i++) {
        extent_array[i] = 0;
    }

    //Counting the frequency of the alphabet
    unsigned char ch_tmp;

    while ((ch = getc(input_fp)) != EOF) {
        ch_tmp = ch;
        extent_array[ch_tmp] = extent_array[ch_tmp] + 1;
    }

    int unique_symbols = 0;
    for (int i = 0; i < ALPHABET; i++) {
        if (extent_array[i] > 0) {
            unique_symbols += 1;
        }
    }

    // 3. Construct the Huffman tree using a priority queue. This will be done using build_tree()
    Node *root = build_tree(extent_array);

    /* 4.  Construct a code table.Each index of the table represents a symbol and the value at that index the
        symbol’s code. You will need to use a stack of bits and perform a traversal of the Huffman tree.
        */

    Code table[ALPHABET];
    build_codes(root, table);

    //5. Construct a header.

    /* typedef struct {
            uint32_t magic;
            uint16_t permissions;
            uint16_t tree_size;
            uint64_t file_size;
        } Header;*/

    header.magic = MAGIC;
    // header.permissions =  0600;
    header.tree_size = (3 * unique_symbols) - 1;
    header.file_size = get_filesize(input_filename);

    //6. Write the constructed header to outfile.

    // int filedesc = open(output_filename, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    int filedesc = open(output_filename, O_CREAT | O_TRUNC | O_WRONLY, header.permissions);
    write(filedesc, &header, sizeof(header));
    if (fchmod(filedesc, header.permissions) == -1) {
        perror("Error chmod");
    }
    close(filedesc);

    //  test input is same as output
    Header header_out;
    int infile = open(output_filename, O_RDONLY);
    read(infile, &header_out, sizeof(header_out));
    close(infile);

    //Checking Header
    assert(header.magic == header_out.magic);
    assert(header.permissions == header_out.permissions);
    assert(header.tree_size == header_out.tree_size);
    assert(header.file_size == header_out.file_size);

    //7. Write the constructed Huffman tree to outfile using dump_tree().
    int filetree = open(output_filename, O_WRONLY | O_APPEND);
    //printf("filetree2 %d \n ",filetree);
    dump_tree(filetree, root);

    // 8. Starting at the beginning of infile, write the corresponding code for each symbol to outfile with write_code().
    //When finished with all the symbols, make sure to flush any remaining buffered codes with flush_codes()

    fseek(input_fp, 0, SEEK_SET);
    bytes_read = 0;
    bytes_written = 0;
    Code table_ch = code_init();
    while ((ch = getc(input_fp)) != EOF) { //keep getting characters until EOF
        table_ch = table[(uint8_t) ch];
        write_code(filetree, &table_ch); //write a code of a character
    }
    flush_codes(filetree);

    if (verbose) {
        //100 × (1 − (compressed size/uncompressed size)).
        uint64_t outfilesize = get_filesize(output_filename);
        float compression = get_spacesaving(header.file_size, outfilesize);
        char chper = ' ';
        fprintf(stderr, "Uncompressed file size: %" PRIu64 " bytes\n", header.file_size);
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes\n", outfilesize);
        fprintf(stderr, "Space saving %.2f%c", compression, chper);
    }

    //9. Close infile and outfile.
    close(input_file);
    close(filetree);

    delete_tree(&root);
    return 0;
}
