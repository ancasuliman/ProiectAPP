#ifndef HOMEWORK_H
#define HOMEWORK_H

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

/* Antet functii */
void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image *out);

#endif /* HOMEWORK_H */