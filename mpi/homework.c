#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Structura pentru pixel (imagine color) */
typedef struct {
    unsigned char r, g, b;
} pixel;

/* Structura pentru imagine de tip pnm sau pgm */
typedef struct {
    char P;
    int height, width;
    unsigned char maxVal;
    pixel **color; 
    unsigned char **bw;     
} image;

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

/* Functie folosita pentru a retine datele din imaginea de input */
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

int main (int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int nProc;      
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);

    int i, j, k, l, low, high, intervalSize;
    float sum, sum_r, sum_g, sum_b;
    int resize_factor;

    // agrv[1] input
	// argv[2] output
	// argv[3] resize_factor
    if(argc < 4) {
		exit(-1);
	}
    image input;
    image output;
    resize_factor = atoi(argv[3]);

    if (rank == 0) {
        readInput(argv[1], &input);
        
        /* Retin header-ul imaginii */
        output.P = input.P;
        output.height = input.height / resize_factor;
        output.width = input.width / resize_factor;
        output.maxVal = input.maxVal;

        /* Aloc memorie */
        if (input.P == '5') {
            output.bw = allocation_bw (output.height, output.width);
        }
        else if (input.P == '6') {
            output.color = allocation_color (output.height, output.width);        
        }

        input.height -= input.height % resize_factor;
        input.width -= input.width % resize_factor;

        for (i = 1; i < nProc; i++) {
            MPI_Send(&input.height, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&input.width, 1, MPI_INT, i, 0, MPI_COMM_WORLD);    
            MPI_Send(&input.P, 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);    

            /* Le trimit celorlalte procese partea lor din matrice */ 
            intervalSize = ceil ((float) (input.height / nProc)); 
            low = i * intervalSize;
            if (i == nProc - 1)
                high = input.height;
            else 
                high = (i + 1) * intervalSize;

            if (input.P == '5') {
                for (j = low; j < high; j++) {
                    MPI_Send(input.bw[j], input.width, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
                }
            }
            else if (input.P == '6') {
                 for (j = low; j < high; j++) {
                    MPI_Send(input.color[j], input.width * 3, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
                }               
            }
        }  
    }
    else {
        MPI_Recv(&input.height, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&input.width, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
        MPI_Recv(&input.P, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);        

        intervalSize = ceil ((float) (input.height / nProc));                
        low = rank * intervalSize;
        if (rank == nProc - 1)
            high = input.height;
        else 
            high = (rank + 1) * intervalSize;

        output.height = input.height / resize_factor;
        output.width = input.width / resize_factor;

        /* Aloc memorie */
        if (input.P == '5') {
            input.bw = allocation_bw (input.height, input.width);
            output.bw = allocation_bw (output.height, output.width);            
            for (i = low; i < high; i++) {
                MPI_Recv(input.bw[i], input.width, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);            
            }
        }
        else if (input.P == '6') {
            input.color = allocation_color (input.height, input.width); 
            output.color = allocation_color (output.height, output.width);              
            for (i = low; i < high; i++) {
                MPI_Recv(input.color[i], input.width * 3, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);            
            }      
        }

    }

    intervalSize = ceil ((float) (input.height / nProc)); 
        
    low = rank * intervalSize;
    if (rank == nProc - 1)
        high = input.height - 1;
    else 
        high = (rank + 1) * intervalSize;

    /* Daca imaginea este grayscale */
    if (input.P == '5') {
        /* Trimit si primesc linia de vecini care ma intereseaza in functie de rank */
        if (rank != nProc - 1)
            MPI_Send(input.bw[high - 1], input.width, MPI_UNSIGNED_CHAR, rank + 1, 0, MPI_COMM_WORLD);
        if (rank != 0) {
            MPI_Recv(input.bw[low - 1], input.width, MPI_UNSIGNED_CHAR, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);            
            MPI_Send(input.bw[low], input.width, MPI_UNSIGNED_CHAR, rank - 1, 0, MPI_COMM_WORLD);            
        }
        if (rank != nProc - 1)
            MPI_Recv(input.bw[high], input.width, MPI_UNSIGNED_CHAR, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);            
        
        for (i = low; i < high; i = i + resize_factor) {
            for (j = 0; j < input.width; j = j + resize_factor) {
                sum = 0;
                for (k = i; k < i + resize_factor; k++) {
                    for (l = j; l < j + resize_factor; l++) {
                        sum += input.bw[k][l];
                    }
                }
            output.bw[i / resize_factor][j / resize_factor] = sum / pow(resize_factor, 2);
            }
        }
    }
    /* Daca imaginea este color */
    else if (input.P == '6') {
        /* Trimit si primesc linia de vecini care ma intereseaza in functie de rank */            
        if (rank != nProc - 1)
            MPI_Send(input.color[high - 1], input.width * 3, MPI_UNSIGNED_CHAR, rank + 1, 0, MPI_COMM_WORLD);
        if (rank != 0) {
            MPI_Recv(input.color[low - 1], input.width * 3, MPI_UNSIGNED_CHAR, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);            
            MPI_Send(input.color[low], input.width * 3, MPI_UNSIGNED_CHAR, rank - 1, 0, MPI_COMM_WORLD);            
        }
        if (rank != nProc - 1)
            MPI_Recv(input.color[high], input.width * 3, MPI_UNSIGNED_CHAR, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);            
        
        for (i = low; i < high; i = i + resize_factor) {
            for (j = 0; j < input.width; j = j + resize_factor) {
                sum_r = 0;
                sum_g = 0;
                sum_b = 0;                
                for (k = i; k < i + resize_factor; k++) {
                    for (l = j; l < j + resize_factor; l++) {
                        sum_r += input.color[k][l].r;
                        sum_g += input.color[k][l].g;
                        sum_b += input.color[k][l].b;                        
                    }
                }
                output.color[i / resize_factor][j / resize_factor].r = sum_r / pow(resize_factor, 2);
                output.color[i / resize_factor][j / resize_factor].g = sum_g / pow(resize_factor, 2);
                output.color[i / resize_factor][j / resize_factor].b = sum_b / pow(resize_factor, 2);
            }
        }
    }
    
    /* Trimit partea din matrice corespunzatoare dupa ce i-am aplicat filtrele */
    if (rank > 0) {
        intervalSize = ceil ((float) (output.height / nProc)); 
        low = rank * intervalSize;
        if (rank == nProc - 1)
            high = output.height;
        else 
            high = (rank + 1) * intervalSize;

        if (input.P == '5') {
            for (j = low; j < high; j++) {
                MPI_Send(output.bw[j], output.width, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }
        else if (input.P == '6') {
            for (j = low; j < high; j++) {
                MPI_Send(output.color[j], output.width * 3, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
            }               
        }
    }
    /* Primul proces primeste modificarile facute de celelalte procese si afiseaza
    rezultatul */
    else {
        for (i = 1; i < nProc; i++) {
            intervalSize = ceil ((float) (output.height / nProc)); 
            low = i * intervalSize;
            if (i == nProc - 1)
                high = output.height;
            else 
                high = (i + 1) * intervalSize;

            if (input.P == '5') {
                for (j = low; j < high; j++) {
                    MPI_Recv(output.bw[j], output.width, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }
            else if (input.P == '6') {
                for (j = low; j < high; j++) {
                    MPI_Recv(output.color[j], output.width * 3, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }               
            }
        }
	    writeData(argv[2], &output);
    }

    MPI_Finalize();
}
