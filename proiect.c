#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#define BUFSIZE 2048


typedef struct Img {
    int height;
    int width;
}Img;

Img imagine;

char *getFileType(char *filename) {
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

char *getFileFromDir(char *filename) {
    char *dot = strrchr(filename, '/');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

bool isBMP(char *filename) {
    if (strcmp(getFileType(filename), "bmp") == 0) return 1;
    return 0;
}

void readBMPInfo(char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) perror("Eroare la deschidere bmp");

    int rd;
    char buff[BUFSIZE];
    rd = read(fd, buff, BUFSIZE);
    if (rd < 0) perror("Eroare la citirea header-ului");
    memcpy(&imagine.width, buff + 18, 4);
    memcpy(&imagine.height, buff + 22, 4);
    close(fd);
}

void convertPixelsToGrey(char *filename) {
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("Eroare la deschidere bmp");
        exit(EXIT_FAILURE);
    }

    Img imgInfo;
    lseek(fd, 18, SEEK_SET);
    if (read(fd, &imgInfo.width, sizeof(int)) < 0 || read(fd, &imgInfo.height, sizeof(int)) < 0) {
        perror("Eroare la citirea dimensiunilor imaginii");
        exit(EXIT_FAILURE);
    }

    lseek(fd, 54, SEEK_SET);

    char pixel[3];
    for (int i = 0; i < imgInfo.width * imgInfo.height; i++) {
        if (read(fd, pixel, sizeof(char) * 3) < 0) {
            perror("Eroare la citire pixel");
            exit(EXIT_FAILURE);
        }

        unsigned char P_gri = 0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2];
        pixel[0] = pixel[1] = pixel[2] = P_gri;

        if (write(fd, pixel, sizeof(char) * 3) < 0) {
            perror("Eroare la scriere pixeli");
            exit(EXIT_FAILURE);
        }
    }
}

DIR *openDir(char *dir) {
    DIR *d = opendir(dir);
    if (d == NULL) perror("Eroare la deschidere fisier!");
    return d;
    closedir(d);
}

void printToFile(char *file, char *content) {
    int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1) {
        perror("Eroare la deschidere fisier");
        close(fd);
        exit(-1);
    }

    if (write(fd, content, strlen(content)) == -1) {
        perror("Eroare la scriere");
        close(fd);
        exit(-1);
    }
    close(fd);
}

int countLines(char *file) {
    int fd = open(file, O_RDONLY);
        if (fd == -1) {
        perror("Eroare la deschiderea fisierului");
        exit(EXIT_FAILURE);
    }
    int count = 0;
    int rd;
    char buff[1024];
    while ((rd = read(fd, buff, sizeof(buff))) > 0 ) {
        for (int i = 0; i < rd; i++) {
            count++;
        }
    }
    close(fd);
    return count;
}

void fileInfo(char *file, char *buffer, struct stat fileStat) {
        if (isBMP(file)) {
            //Fisier BMP
            readBMPInfo(file);
            sprintf(buffer, "Nume fisier: %s\nInaltime: %d\nLungime: %d\n"
                            "dimensiune: %ld\nIdentificatorul utilizatorului: %d\n"
                            "Timpul ultimei modificari: %sContorul de legaturi: %ld\n"
                            "Drepturi usr: %c%c%c\n"
                            "Drepturi grp: %c%c%c\n"
                            "Drepturi others: %c%c%c\n\n", getFileFromDir(file), imagine.height, imagine.width, fileStat.st_size, fileStat.st_uid, ctime(&fileStat.st_mtime),
                            fileStat.st_nlink,
                            (fileStat.st_mode & S_IRUSR) ? 'R' : '-',
                    (fileStat.st_mode & S_IWUSR) ? 'W' : '-',
                    (fileStat.st_mode & S_IXUSR) ? 'X' : '-',
                    (fileStat.st_mode & S_IRGRP) ? 'R' : '-',
                    (fileStat.st_mode & S_IWGRP) ? 'W' : '-',
                    (fileStat.st_mode & S_IXGRP) ? 'X' : '-',
                    (fileStat.st_mode & S_IROTH) ? 'R' : '-',
                    (fileStat.st_mode & S_IWOTH) ? 'W' : '-',
                    (fileStat.st_mode & S_IXOTH) ? 'X' : '-');
        } else {
            //Fisier fara .bmp
            sprintf(buffer, "Nume fisier: %s\n"
                            "dimensiune: %ld\nIdentificatorul utilizatorului: %d\n"
                            "Timpul ultimei modificari: %sContorul de legaturi: %ld\n"
                            "Drepturi usr: %c%c%c\n"
                            "Drepturi grp: %c%c%c\n"
                            "Drepturi others: %c%c%c\n\n", getFileFromDir(file), fileStat.st_size, fileStat.st_uid, ctime(&fileStat.st_mtime),
                    fileStat.st_nlink,
                    (fileStat.st_mode & S_IRUSR) ? 'R' : '-',
                    (fileStat.st_mode & S_IWUSR) ? 'W' : '-',
                    (fileStat.st_mode & S_IXUSR) ? 'X' : '-',
                    (fileStat.st_mode & S_IRGRP) ? 'R' : '-',
                    (fileStat.st_mode & S_IWGRP) ? 'W' : '-',
                    (fileStat.st_mode & S_IXGRP) ? 'X' : '-',
                    (fileStat.st_mode & S_IROTH) ? 'R' : '-',
                    (fileStat.st_mode & S_IWOTH) ? 'W' : '-',
                    (fileStat.st_mode & S_IXOTH) ? 'X' : '-');
            
        }
}

void linkInfo(char *file, char *buffer, struct stat fileStat) {
        //Legatura simbolica
        char targetPath[1024];
        int bytesRead = readlink(file, targetPath, sizeof(targetPath) - 1);
        struct stat targetStat;
        if (stat(file, &targetStat) == -1) perror("Eroare la targetStat");
        if (bytesRead == -1) perror("Eroare la citire legatura simbolica");
        targetPath[bytesRead] = '\0';
    
        sprintf(buffer, "Nume legatura: %s->%s\nDimensiune legatura: %ld\nDimensiune fisier: %ld\n"
                        "Drepturi usr: %c%c%c\n"
                        "Drepturi grp: %c%c%c\n"
                        "Drepturi others: %c%c%c\n\n", file, targetPath, fileStat.st_size, targetStat.st_size,
                    (fileStat.st_mode & S_IRUSR) ? 'R' : '-',
                    (fileStat.st_mode & S_IWUSR) ? 'W' : '-',
                    (fileStat.st_mode & S_IXUSR) ? 'X' : '-',
                    (fileStat.st_mode & S_IRGRP) ? 'R' : '-',
                    (fileStat.st_mode & S_IWGRP) ? 'W' : '-',
                    (fileStat.st_mode & S_IXGRP) ? 'X' : '-',
                    (fileStat.st_mode & S_IROTH) ? 'R' : '-',
                    (fileStat.st_mode & S_IWOTH) ? 'W' : '-',
                    (fileStat.st_mode & S_IXOTH) ? 'X' : '-');
}

void dirInfo(char *file, char* buffer, struct stat fileStat) {
        sprintf(buffer,"Nume director: %s\nIdentificatorul utilizatorului: %d\n"
        "Drepturi usr: %c%c%c\n"
        "Drepturi grp: %c%c%c\n"
        "Drepturi others: %c%c%c\n\n", getFileFromDir(file), fileStat.st_uid,
                (fileStat.st_mode & S_IRUSR) ? 'R' : '-',
                (fileStat.st_mode & S_IWUSR) ? 'W' : '-',
                (fileStat.st_mode & S_IXUSR) ? 'X' : '-',
                (fileStat.st_mode & S_IRGRP) ? 'R' : '-',
                (fileStat.st_mode & S_IWGRP) ? 'W' : '-',
                (fileStat.st_mode & S_IXGRP) ? 'X' : '-',
                (fileStat.st_mode & S_IROTH) ? 'R' : '-',
                (fileStat.st_mode & S_IWOTH) ? 'W' : '-',
                (fileStat.st_mode & S_IXOTH) ? 'X' : '-');
    }

void processEntry(char *file, char *dirOut) {
    struct stat fileStat;
    if (lstat(file, &fileStat) == -1) perror("Eroare la stat");
    char buffer[BUFSIZE];
    char outPath[256];
    sprintf(outPath, "%s/%s_statistica.txt", dirOut, getFileFromDir(file));

    if (S_ISREG(fileStat.st_mode)) {
        fileInfo(file, buffer, fileStat);
    } else if(S_ISLNK(fileStat.st_mode)) {
        linkInfo(file, buffer, fileStat);
    } else if(S_ISDIR(fileStat.st_mode)) {
        dirInfo(file, buffer, fileStat);
    }
    printToFile(outPath, buffer);

}

int main(int argc, char* argv[]) {
    char *dirIn = argv[1];
    char *dirOut = argv[2];
    if (argc != 3) perror("Eroare la argumentul primit!");
    DIR *dir = openDir(dirIn);
    if (dir == NULL) perror("Eroare la deschiderea directorului");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp (entry->d_name, "."))
            continue;
        if (!strcmp (entry->d_name, ".."))
            continue;
        char entryPath[BUFSIZE];
        sprintf(entryPath, "%s/%s", dirIn, entry->d_name);

        int pid_statistica = fork();
        int status;
        if (pid_statistica < 0) {
            perror("Eroare la pid statistica");
            exit(-1);
        }

        if (pid_statistica == 0) { //proces copil pid_statistica
            exit(-1);

        } else { 
            //proces parinte pid_statistica
            int pid_pixeli = fork();
            if (pid_pixeli < 0) {
                perror("Eroare la pid pixeli");
                exit(-1);
            }

            if (pid_pixeli == 0) { //proces copil pid_pixeli
                if (isBMP(entryPath)) {
                    convertPixelsToGrey(entryPath);
                }
                exit(-1);

            } else { //proces parinte pid_pixeli
                waitpid(pid_pixeli, &status, 0);
                printf("S-a încheiat procesul cu pid-ul %d și codul %d\n", pid_pixeli, WEXITSTATUS(status));
            }
            processEntry(entryPath, dirOut);
            }
    }
    while (wait(NULL) > 0);
    closedir(dir);
    return 0;
}