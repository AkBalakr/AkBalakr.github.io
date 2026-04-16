
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <string.h>
#include <math.h> //abs(), sqrt()
#include <inttypes.h>
#include <ctype.h>

#include "parser.h"
#include "text.h"
#include "node.h"
#include "ht.h"
#include "bf.h"

#define HT_SIZE 524288 //2^19

uint32_t noiselimit; // Number of noise words to filter out.

struct Text {
    HashTable *ht;
    BloomFilter *bf; //Used for helping making finding faster
    uint32_t word_count; //Total words not unqiue words
};

Text *text_create(FILE *infile, Text *noise) {
    bool make_noise = false;
    uint32_t noise_limit = noiselimit;
    if (noise == NULL) {
        //printf("Making Noise Text\n");
        make_noise = true;
    } else {
        make_noise = false;
        //printf("Making Text\n");
    }

    Text *text = (Text *) malloc(sizeof(Text)); //Allocating memory for Text
    //printf("Memory for Text Malloced\n");
    text->ht = ht_create(HT_SIZE);
    text->bf = NULL;
    text->word_count = 0; //ht_size(text->ht) might belong here
    regex_t re_word;
    regcomp(&re_word, "[a-zA-Z]+['-]?[a-zA-Z]+", REG_EXTENDED);

    //BLOOM
    //printf("Before Bloom\n");
    text->bf = bf_create(HT_SIZE * 4); //2^21 and HT_SIZE is 2^19
    //printf("After Bloom \n");

    char *word;
    //printf("Next_word\n");
    word = next_word(infile, &re_word);
    //printf("First Word %s\n", word);
    while (word != NULL) {
        //Make lowercase
        if (make_noise) {
            noise_limit--;
            if (noise_limit <= 0) {
                break;
            }
        }
        for (uint32_t i = 0; word[i] != '\0'; i++) { //String ends at '\0' regardless of size
            word[i] = tolower(word[i]); //make it lower
        }

        text->word_count++;
        if (make_noise) {
            ht_insert(text->ht, word);

            //printf("Before Bloom Insert\n");
            bf_insert(text->bf, word);
            //printf("After Bloom Insert\n");

        } else if (!make_noise && text_contains(noise, word) && strcmp(word, "")) {
            //printf("--->'%s' NOISE\n", word);

        } else if (
            !make_noise) { //If there is a noise text and current word is not in noise add it to text
            ht_insert(text->ht, word);
            //printf("Before Bloom Insert\n");
            bf_insert(text->bf, word);
            //printf("After Bloom Insert\n");
        }
        word = next_word(infile, &re_word);
    }

    //printf("\n");
    //ht_print(text->ht);
    //printf("text_create Return\n");
    return text;
}

void text_delete(Text **text) {
    bf_delete(&((*text)->bf));
    ht_delete(&((*text)->ht));
    free(*text);
    *text = NULL;
    text = NULL;
}

double text_dist(Text *text1, Text *text2, Metric metric) {
    //printf("\n text_dist text_dist");

    //Somehow get these true falses from the metric input not sure how to yet
    //printf("USE METRIC %u\n", metric);

    HashTableIterator *hti_anon
        = hti_create(text1->ht); //Making a iterator to ... iterate over the ht of Text 1
    HashTableIterator *hti_data
        = hti_create(text2->ht); //Making a iterator to ... iterate over the ht of Text 2
    //Node *node = ht_iter(hti_anon); //Node first for Anon and then for Data

    float dist = 0;
    float increment = 0;
    float anon_freq;
    float data_freq;
    char *word; //Current target word for finding distance
    //printf("Word Count anon(%"PRIu32" data(%"PRIu32"\n", text1->word_count, text2->word_count);

    Node *node = ht_iter(hti_anon); //Node first for Anon and then for Data

    uint32_t test_loops = 0; //FOR TESTING
    while (node != NULL) { //If ht_iter finishes a full loop it returns NULL
        //uint32_t tag = 1; //FOR TESTING
        test_loops++; //FOR TESTING
        word = node->word;

        //printf("\n<==== DIST LOOP %"PRIu32" Word: '%s' ====>\n", test_loops, word); //FOR TESTING
        //printf("File 1");
        anon_freq
            = text_frequency(text1, word); //-1 if there is no node for the word in text1(anon)
        //printf("File 2");
        //printf("Anon F(%f) - ", anon_freq);
        data_freq
            = text_frequency(text2, word); //-1 if there is no node for the word in text2(data)

        //printf("Data F(%f) = ", data_freq);
        //printf("%f\n", (anon_freq - data_freq));

        if (metric == 1) { //|ui - vi| Manhattan
            increment = fabs(anon_freq - data_freq);
        } else if (metric == 0) { //sqrt(ui - vi)^2 Euclidean
            increment = fabs((anon_freq - data_freq) * (anon_freq - data_freq));
        } else if (metric == 2) { //1 - (ui * vi) Cosine
            increment = fabs(anon_freq * data_freq);
        }

        if (!strcmp(word, "")) {
            increment = 0;
        }
        dist += fabs(increment);
        node = ht_iter(hti_anon);
        if (node == NULL) { //If all anon iterator done go to data iterator
            node = ht_iter(hti_data);
        }
        //printf("%f\n", dist);
    }

    //node_delete(&node);
    //Free iterators
    hti_delete(&hti_anon);
    hti_delete(&hti_data);
    if (metric == 0) { //sqrt(ui^2 - vi^2) Euclidean
        dist = sqrt(dist);
    } else if (metric == 2) { //1 - (ui * vi) Cosine
        dist = 1 - dist;
    }
    return dist;
}

double text_frequency(
    Text *text, char *word) { //Would have been easier if had a node_frequency function
    //printf("Test frequency ");
    double frequency = 0;

    bool bloom = bf_probe(text->bf, word);
    //printf("text_contains - word = %s bloom  = %i \n\n ", word, bloom);

    if (bloom) {
        //ht_print(text->ht);
        Node *node = ht_lookup(text->ht, word);
        frequency = node->count;
        //printf("frequncy start");
        //printf("f(%d) ",(int)frequency);
        //node_delete(&node);
    }

    return frequency
           / text->word_count; //Divide by the total word count because the math people said so
}

bool text_contains(Text *text, char *word) {
    //BLOOM FILTER CHECK GOES HERE FIRST
    bool bloom = bf_probe(
        text->bf, word); //If bloom is false word is not there, if true check for false positive
    //printf("text_contains - word = %s bloom  = %i \n\n ", word, bloom);
    if (!bloom)
        return false;

    bool contains = false; //false until true, anti-justice
    HashTableIterator *hti = hti_create(text->ht);
    Node *node = ht_iter(hti);
    while (node != NULL && bloom) { //If there is more iterations and contain not true keep checking
        if (!(strcmp(node->word, word))) {
            contains = true; //Found it!
            break; //should be unncessary but just incase
        }
        node = ht_iter(hti); //node is the next valid node
    }

    hti_delete(&hti);
    return contains;
}

void text_print(Text *text);
