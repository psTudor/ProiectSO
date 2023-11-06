#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


typedef struct Img {
    char numeFisier[20];
    int inaltime;
    int lungime;
    int dimensiune;
    char usrID[10];
    char lastEdit[10];
    int count;
    char acUsr[3];
    char acGrp[3];
    char acAltii[3];
}Img;

char *getFileType(char *filename) {
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}


int main(int argc, char* argv[]) {
    struct stat fisType;
    if (argc != 2) perror("Numar de argumente gresit!");
    stat(argv[1], &fisType);
    printf("%F\n", fisType.st_mode);
    return 0;
}