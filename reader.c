#include "header.h"

char *read_file(char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open input file.\n");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long int size = ftell(fp);


    char *buf = calloc(1, size + 2);
    fseek(fp, 0, SEEK_SET);

    fread(buf, size, 1, fp);

    if(size == 0 || buf[size - 1] != '\n') {
        buf[size++] = '\n';
    }
    buf[size] = '\0';
    fclose(fp);
    return buf;
}