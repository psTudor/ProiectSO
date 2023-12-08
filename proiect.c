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

char *getFilenameFromDir(char *dirname) {
    char *dot = strrchr(dirname, '/');
    if (!dot || dot == dirname) return "";
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
        exit(EXIT_FAILURE);
    }

    if (write(fd, content, strlen(content)) == -1) {
        perror("Eroare la scriere");
        close(fd);
        exit(EXIT_FAILURE);
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
            if (buff[i] == '\n') {
                count++;
            }
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
                            "Drepturi others: %c%c%c\n\n", getFilenameFromDir(file), imagine.height, imagine.width, fileStat.st_size, fileStat.st_uid, ctime(&fileStat.st_mtime),
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
                            "Drepturi others: %c%c%c\n\n", getFilenameFromDir(file), fileStat.st_size, fileStat.st_uid, ctime(&fileStat.st_mtime),
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
        "Drepturi others: %c%c%c\n\n", getFilenameFromDir(file), fileStat.st_uid,
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
    if (lstat(file, &fileStat) == -1) {
        perror("Eroare la stat");
        exit(EXIT_FAILURE);
    }
    char buffer[BUFSIZE];
    char outPath[256];
    sprintf(outPath, "%s/%s_statistica.txt", dirOut, getFilenameFromDir(file));
    
    if (S_ISREG(fileStat.st_mode)) {
        fileInfo(file, buffer, fileStat); //functie care scrie detalii despre fisier normal sau .bmp
    } else if(S_ISLNK(fileStat.st_mode)) {
        linkInfo(file, buffer, fileStat); //functie care scrie detalii despre legatura simbolica
    } else if(S_ISDIR(fileStat.st_mode)) {
        dirInfo(file, buffer, fileStat);  //functie care scrie detalii despre un director
    }
    printToFile(outPath, buffer);
}

int getFileSize(int fd) {
    int fileSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    return fileSize;
}

void fileContent(char *file, char *buffer) {
    int fd = open(file, O_RDONLY);

    if (fd < 0) {
        perror("Eroare la deschidere fisier");
        exit(EXIT_FAILURE);
    }
    int size = getFileSize(fd);
    int rd;
    if ((rd = read(fd, buffer, size)) < 0) {
        perror("Eroare la sciere in buffer");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    char *dirIn = argv[1];
    char *dirOut = argv[2];
    char c[2];
    strcpy(c, argv[3]);
    if (argc != 4) perror("Eroare la argumentul primit!");
    DIR *dir = openDir(dirIn);
    if (dir == NULL) perror("Eroare la deschiderea directorului");
    
    struct dirent *entry;
    int pfd_ff[2]; // pipe de la fiu la fiu
    int pfd_fp[2]; // pipe de la fiu la parinte
    char bufferWrite[BUFSIZE];
    char bufferRead[500];
    char entryPath[BUFSIZE];
    struct stat fileStat;

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp (entry->d_name, "."))
            continue;
        if (!strcmp (entry->d_name, ".."))
            continue;
        sprintf(entryPath, "%s/%s", dirIn, entry->d_name);

        if (lstat(entryPath, &fileStat) == -1) {
            perror("Eroare la stat");
            exit(EXIT_FAILURE);
        }

        if (pipe(pfd_ff) < 0) {
            perror("Eroare la pipe");
            exit(EXIT_FAILURE);
        }


        if (pipe(pfd_fp) < 0) {
            perror("Eroare la pipe");
            exit(EXIT_FAILURE);
        }

        int pid = fork();
        if (pid < 0) {
            perror("Eroare la pid statistica");
            exit(EXIT_FAILURE);
        }

        if (!isBMP(entryPath) && !S_ISDIR(fileStat.st_mode)) {
            if (pid == 0) { //proces fiu statistica
                processEntry(entryPath, dirOut); //functie care creeaza fisierul de statistica in directorul de iesire
                fileContent(entryPath, bufferWrite);

                close(pfd_ff[0]);
                close(pfd_fp[0]);
                close(pfd_fp[1]);
                if (write(pfd_ff[1], bufferWrite, sizeof(bufferWrite)) < 0) {
                    perror("Eroare la scriere in pipe");
                    exit(EXIT_FAILURE);
                }
                close(pfd_ff[1]);
                exit(EXIT_SUCCESS);
            }

            //proces printe statistica
            waitpid(pid, NULL, 0);
            if ((pid = fork()) < 0) {
                perror("Eroare la pid statistica");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) { //fiu propozitii corecte
                close(pfd_ff[1]);
                close(pfd_fp[0]);

                dup2(pfd_ff[0], 0);
                dup2(pfd_fp[1], 1);

                close(pfd_ff[0]);
                close(pfd_fp[1]);

                execlp("sh", "sh", "script.sh", c, (char *)NULL);
                perror("Eroare la execlp");
                exit(EXIT_FAILURE);
            }

            //proces parinte propozitii
            close(pfd_ff[0]);
            close(pfd_ff[1]);
            close(pfd_fp[1]);
            waitpid(pid, NULL, 0);
            if ((read(pfd_fp[0], bufferRead, sizeof(bufferRead))) < 0) {
                perror("Eroare la citire din pipe");
                exit(EXIT_FAILURE);   
            }
            printf("%s in fisierul %s\n", bufferRead, getFilenameFromDir(entryPath));
            close(pfd_fp[0]);
        }

        if (isBMP(entryPath)) {
            processEntry(entryPath, dirOut);
            int pid_pixeli = fork();
            if (pid_pixeli < 0) {
                perror("Eroare la pid statistica");
                exit(EXIT_FAILURE);
            }
            if (pid_pixeli ==  0) { //proces fiu pid_pixeli
                convertPixelsToGrey(entryPath);
                exit(EXIT_SUCCESS);
            }
            int pidf, status;
            if ((pidf = wait(&status)) < 0) {
                perror("Eroare la wait");
                exit(EXIT_FAILURE);
            }
            if (WIFEXITED(status)) {
                printf("S-a încheiat procesul cu pid-ul %d și codul %d\n", pidf, WEXITSTATUS(status));
            }
        }
    }
    closedir(dir);
    return 0;
}