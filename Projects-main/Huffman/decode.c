#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "header.h"
#include "node.h"
#include "huffman.h"
#include "io.h"

void postorder_print(Node *root) {
    if (root != NULL) {
        postorder_print(root->left);
        postorder_print(root->right);
        printf(" %c->f(%" PRIu64 ") \n", root->symbol, root->frequency);
    }
}

float get_spacesaving(int inputsize, int outsize) {

    return 100 * (1 - ((float) outsize / (float) inputsize));
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

void usage(void) {
    fprintf(stderr, "usage: encrypt [-hiov]\n");
    fprintf(stderr, "-h : Prints out a help message describing the purpose of the program \t\n "
                    "-i:Specifies the input file to decode using Huffman coding.The default input "
                    "should beset as stdin.t\n-o outfile : Specifies the output file to write the "
                    "decompressed input to. The default output should be set as stdout. \t\n-v : "
                    "Prints decompression statistics to stderr.\t\n");
    exit(EXIT_FAILURE);
}
int main(int argc, char **argv) {
    //FILE *public_fp;
    //mFILE *output_fp = stdout;

    char *input_filename = "files/encrypt_out.txt";
    char *output_filename = "files/decrypt_out.txt";
    bool input_file = false;
    bool output_file = false;
    bool verbose = false;
    int ch;
    while ((ch = getopt(argc, argv, "i:o:n:vh")) != -1) {
        switch (ch) {
        case 'h': //displays program synopsis and usage.
            usage();
            break;
        case 'i': //Specifies the input file to decode using Huffman coding.The default input should be set as stdin
            input_filename = optarg;
            input_file = true;
            break;
        case 'o':
            output_filename
                = optarg; //Specifies the output file to write the decompressed input to. The default output should be set as stdout.
            output_file = true;
            break;
        case 'v': //Prints decompression statistics to stderr.
            verbose = true;
            break;
        default: break;
        }
    }

    /* Read in the header from infile and verify the magic number. If the magic number does not match 0xBEEFBBAD (defined as MAGIC in defines.h), 
   then an invalid file was passed to your program. Display a helpful error message and quit. */

    Header header_out;
    int decrypt_infile = open(input_filename, O_RDONLY);
    read(decrypt_infile, &header_out, sizeof(header_out));

    //printf("header_out.magic  %" PRIu32 " %X \n", header_out.magic, header_out.magic);
    //printf("header_out.permissions  %" PRIu16 "\n", header_out.permissions);
    //printf("header_out.tree_size  %" PRIu16 "\n", header_out.tree_size);
    //printf("header_out.file_size  %" PRIu64 "\n", header_out.file_size);

    //2. The permissions field in the header indicates the permissions that out file should be set to.Set the permissions using fchmod().

    int filedesc = open(output_filename, O_CREAT | O_TRUNC | O_WRONLY, header_out.permissions);
    if (fchmod(filedesc, (mode_t) header_out.permissions) == -1) {
        perror("Error chmod");
    }

    /*
    The size of the dumped tree is given by the tree_size field in the header. 
    Read the dumped tree from infile into an array that is tree_size bytes long. 
    Then, reconstruct the Huffman tree using rebuild_tree().
    */

    uint16_t nbytes = header_out.tree_size;
    uint8_t tree[nbytes];
    read(decrypt_infile, tree, nbytes);

    Node *node_rebuild = rebuild_tree(nbytes, tree);

    /*
4. Read infile one bit at a time using read_bit(). You will be traversing down the tree one link at a time for each bit that is read.
(a) Begin at the root of the Huffman tree. If a bit of value 0 is read, then walk down to the left child of the current node. 
    Else, if a bit of value 1 is read, then walk down to the right child of the current node.
(b) If you find yourself at a leaf node, then write the leaf node’s symbol to outfile. Note: you may alternatively buffer 
    these symbols and write out the buffer whenever it is filled (this will be more efficient). 
    After writing the symbol, reset the current node back to the root of the tree.
(c) Repeat until the number of decoded symbols matches the original filesize ,which is given by the file_size field in the header
    that was read from infile
*/
    uint8_t byte; // Stores a byte
    uint8_t bit; // Stores a bit of byte
    uint8_t buffer[1] = "\0";
    uint64_t f_size = 0;
    bool read_bit_flag = true;
    bytes_read = 0;
    bytes_written = 0;
    uint32_t bytes = 0;

    Node *node = node_rebuild;
    while (f_size < header_out.file_size) {
        read_bit_flag = read_bit(decrypt_infile, &byte);
        //printf("\n");
        while ((!read_bit_flag) && (f_size < header_out.file_size)) {
            for (uint32_t b = 8; b > 0; b--) {

                bit = ((byte >> (b - 1)) & 1);
                //printf("%"PRIu16"", bit);
                if (f_size < header_out.file_size) {
                    if (node != NULL) {
                        //printf("%"PRIu16"", bit);
                        if (node->left == NULL && node->right == NULL) { // No Children = Leaf
                            buffer[0] = node->symbol;
                            write_bytes(filedesc, &(buffer[0]), sizeof(buffer));
                            //write_bytes(filedesc, &(buffer[bytes]), sizeof(char));
                            bytes += 1;
                            bytes %= 8;
                            f_size++;
                            node = node_rebuild; // Reset Node to Root Nod
                            //printf(" %c \n", node->symbol);
                        }
                        if (bit == 0 && node->left != NULL) { // 0 -> Left
                            node = node->left;

                        } else if (bit == 1 && node->right != NULL) { // 1 -> Right
                            node = node->right;
                        }
                    }
                }
            }
            read_bit_flag = read_bit(decrypt_infile, &byte);
        }
    }

    if (verbose) {
        //100 × (1 − (compressed size/uncompressed size)).
        uint64_t outfilesize = get_filesize(output_filename);
        float compression = get_spacesaving(header_out.file_size, bytes_written);
        char chper = ' ';
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes\n", header_out.file_size);
        fprintf(stderr, "Uncompressed file size: %" PRIu64 " bytes\n", outfilesize);
        fprintf(stderr, "Space saving %.2f%c", compression, chper);
    }

    close(filedesc);
    close(decrypt_infile);
    node = node_rebuild;
    delete_tree(&node); //Deleting the tree Post Order
}
