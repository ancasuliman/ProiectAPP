#ifndef HOMEWORK_H
#define HOMEWORK_H

/* Macrodefinitie pentru elementul minim */
#define MIN(a, b) (a < b? a : b)

/* Structura pentru pixel (imagine color) */
typedef struct {
    unsigned char r, g, b;
} pixel;

/* Structura pentru imagine de tip pnm */
typedef struct {
    char P;
    int height, width;
    unsigned char maxVal;
    pixel **color; 
    unsigned char **bw;     
} image;

/* Structura pentru thread (o dau ca parametru in threadFunction) */
typedef struct {
    image *in;
    image *out;
    int thread_id;
} thread_struct;

/* Antet functii */
void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image * out);

#endif /* HOMEWORK_H */