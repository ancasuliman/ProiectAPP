#include "homework.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

int num_threads;
int resize_factor;

/* Functie care imi aloca o matrice cu elemente de tip pixel */
pixel **allocation_color(int height, int width) {
    int i, j;
    pixel **img;
    
    img = malloc (height * sizeof(pixel *));
    if (img == NULL) 
        return NULL;

    for (i = 0; i < height; i++) {
        img[i] = malloc (width * sizeof(pixel));
        if (img[i] == NULL) {
            for (j = 0; j < i; j++)
                free (img[j]);
            free(img);
            return NULL;
        }
    }
    return img;
}

/* Functie folosita pentru eliberarea memoriei */
void free_color (pixel ***img, int height) {
    int i;
    for (i = 0; i < height; i++) 
        free((*img)[i]);
    free(*img);
    *img = NULL;
}

/* Functie care imi aloca o matrice in care retin pixelii pentru imaginea 
    grayscale */
unsigned char **allocation_bw(int height, int width) {
    int i, j;
    unsigned char **img;
    
    img = malloc (height * sizeof(unsigned char *));
    if (img == NULL) 
        return NULL;

    for (i = 0; i < height; i++) {
        img[i] = malloc (width * sizeof(unsigned char));
        if (img[i] == NULL) {
            for (j = 0; j < i; j++)
                free (img[j]);
            free(img);
            return NULL;
        }
    }
    return img;
}

/* Functie folosita pentru eliberarea memoriei */
void free_bw (unsigned char ***img, int height) {
    int i;
    for (i = 0; i < height; i++) 
        free((*img)[i]);
    free(*img);
    *img = NULL;
}

/* Functie folosita pentru a retine datele din imagine de input */
void readInput(const char *fileName, image *img) {
    int i;
    /* Deschid fisierul */
    FILE *file_in = fopen(fileName, "rb");

    /* Sar peste primul byte */
    fseek(file_in, 1, SEEK_CUR);
    /* Retin tipul imaginii */
    fread(&img -> P, sizeof(char), 1, file_in);

    /* Sar peste '\n' */
    fseek(file_in, 1, SEEK_CUR);
    /* Retin dimensiunile imaginii */
    fscanf(file_in, "%d", &img -> width);
    fscanf(file_in, "%d", &img -> height);

    /* Sar peste '\n' */
    fseek(file_in, 1, SEEK_CUR);
    /* Retin maxVal (maxim 255) */
    fscanf(file_in, "%hhu", &img -> maxVal);

    /* Sar peste '\n' */
    fseek(file_in, 1, SEEK_CUR);
    
    /* In functie de tipul imaginii (color sau grayscale) */
    if (img -> P == '5') {              // grayscale
        img -> bw = allocation_bw (img -> height, img -> width);
        for (i = 0; i < img -> height; i++) 
            fread(img -> bw[i], sizeof(unsigned char), img -> width, file_in);
    }

    else if (img -> P == '6') {         // color
        img -> color = allocation_color (img -> height, img -> width);
        for (i = 0; i < img -> height; i++) 
            fread(img -> color[i], sizeof(pixel), img -> width, file_in);
    }

    /* Inchid fisierul */ 
    fclose(file_in);
}

/* Functie folosita pentru a crea imaginea de output */
void writeData(const char *fileName, image *img) {
    int i;
    /* Deschid fisierul */ 
    FILE *file_out = fopen(fileName, "wb");

    /* Creez header-ul output-ului */
    fprintf(file_out, "P%c\n", img -> P);
    fprintf(file_out, "%d %d\n", img -> width, img -> height);
    fprintf(file_out, "%hhu\n", img -> maxVal);
    
    /* In functie de tipul imaginii (color sau grayscale) */
    if (img -> P == '5') {              // grayscale
        for (i = 0; i < img -> height; i++)
            fwrite(img -> bw[i], sizeof(unsigned char), img -> width, file_out);
        free_bw(&img -> bw, img -> height);
    }

    else if (img -> P == '6') {         // color
        for (i = 0; i < img -> height; i++)
            fwrite(img -> color[i], sizeof(pixel), img -> width, file_out);   
        free_color(&img -> color, img -> height);     
    }
    /* Inchid fisierul */
    fclose(file_out);
}

void *threadFunction (void *var) {

    thread_struct thread = *(thread_struct*) var;
    int i, j, k, l, sum, sum_r, sum_g, sum_b, low, high, intervalSize;

    /* Creez capetele intervalului pentru fiecare thread */
    intervalSize = ceil ((float) (thread.out -> height) / num_threads) * 
                                                            resize_factor;
    low = thread.thread_id * intervalSize;
    high = MIN ((thread.thread_id + 1) * intervalSize, thread.in -> height);    

    /* Daca imaginea este grayscale */
    if (thread.in -> P == '5') {
        for (i = low; i < high; i = i + resize_factor) {
            for (j = 0; j < thread.in -> width; j = j + resize_factor) {
                sum = 0;
                for (k = i; k < i + resize_factor; k++) {
                    for (l = j; l < j + resize_factor; l++) {
                        sum += thread.in -> bw[k][l];
                    }
                }
                thread.out->bw[i / resize_factor][j / resize_factor] = 
                                                sum / pow(resize_factor, 2);
            }
        }
    }
    /* Daca imaginea este color */
    else if (thread.in -> P == '6') {
        for (i = low; i < high; i = i + resize_factor) {
            for (j = 0; j < thread.in -> width; j = j + resize_factor) {
                sum_r = 0;
                sum_g = 0;
                sum_b = 0;
                for (k = i; k < i + resize_factor; k++) {
                    for (l = j; l < j + resize_factor; l++) {
                        sum_r += thread.in -> color[k][l].r;
                        sum_g += thread.in -> color[k][l].g;
                        sum_b += thread.in -> color[k][l].b;                        
                    }
                }
                thread.out->color[i / resize_factor][j / resize_factor].r = 
                                                sum_r / pow(resize_factor, 2);
                thread.out->color[i / resize_factor][j / resize_factor].g = 
                                                sum_g / pow(resize_factor, 2);
                thread.out->color[i / resize_factor][j / resize_factor].b = 
                                                sum_b / pow(resize_factor, 2);
            }
        }
    }
    return NULL;
}

void resize(image *in, image *out) { 

    int i;
    pthread_t tid[num_threads];
    /* Creez vector de structuri thread_struct */
    thread_struct *thread = malloc (num_threads * sizeof(thread_struct));

    /* Retin header-ul imaginii */
    out -> P = in -> P;
    out -> height = in -> height / resize_factor;
    out -> width = in -> width / resize_factor;
    out -> maxVal = in -> maxVal;

    /* Aloc memorie */
    if (in -> P == '5') {
        out -> bw = allocation_bw (out -> height, out -> width);
    }
    else if (in -> P == '6') {
        out -> color = allocation_color (out -> height, out -> width);        
    }
    
    /* Completez structura pentru fiecare thread */
    for (i = 0; i < num_threads; i++) {
        thread[i].in = in;
        thread[i].thread_id = i;
        thread[i].out = out;
    }

    in -> height -= in -> height % resize_factor;
    in -> width -= in -> width % resize_factor;

    for (i = 0; i < num_threads; i++) {
        pthread_create(&(tid[i]), NULL, threadFunction, &(thread[i]));
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(tid[i], NULL);
    }

    for (i = 0; i < num_threads; i++)     
        out = thread[i].out;

    /* Eliberez memoria */
    if (in -> P == '5') {
        free_bw(&in -> bw, in -> height);
    }
    else {
        free_color(&in -> color, in -> height);
    }
    free(thread);
}
