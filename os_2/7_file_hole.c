/*
Ovaj program demonstrira kreiranje rupa u fajlu.
Vise o rupama u fajlu u TLPI (poglavlje 4.7).

Napomena:
Nakon poziva funkcije create_hole, mozete ispisati sadrzaj
fajla kroz terminal na vise nacina:
- less 1.txt
- cat 1.txt
- od -c 1.txt

Poziv programa:
./filehole filename
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define check_error(cond,msg)\
  do {\
    if (!(cond)) {\
      perror(msg);\
      fprintf(stderr, "File: %s\nFunction: %s\nLine: %d\n", __FILE__, __func__, __LINE__);\
      exit(EXIT_FAILURE);\
    }\
  } while (0)

bool os_file_open(const char *filePath, const char *mode, int *fd) {
    /*
     * Privilegije se postavljaju na rw-r--r-- (0644)
     */
    static mode_t defaultMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    
    /*
	 * Postupno se dodaju flagovi za open funkciju
     */
    int flags = 0;
    switch (mode[0]) {
    case 'r':
		/* u slucaju citanja ukljucuje se:
		 * O_RDONLY - samo citanje
		 * O_RDWR - kombinovano citanje i pisanje, fajl offset na pocetku, ako se ne pise po fajlu
		 * 			sadrzaj ce biti nepromenjen
		 */
        flags |= '+' == mode[1] ? O_RDWR : O_RDONLY;
        break;
    case 'w':
		/* u slucaju citanja ukljucuje se:
		 * O_WRONLY - samo pisanje
		 * O_RDWR - kombinovano citanje i pisanje, fajl offset na pocetku, sadrzaj fajla se brise
		 */
        flags |= '+' == mode[1] ? O_RDWR : O_WRONLY;
        flags |= O_TRUNC;	/* flag kojim se brise sadrzaj fajla */
        flags |= O_CREAT;	/* flag kojim se fajl kreira u slucaju da ne postoji */
        break;
    case 'a':
		/* u nadovezivanja citanja ukljucuje se:
		 * O_WRONLY - samo pisanje
		 * O_RDWR - kombinovano citanje i pisanje, fajl offset na kraju
		 */
        flags |= '+' == mode[1] ? O_RDWR : O_WRONLY;
        flags |= O_APPEND;		/* flag kojim se ukljucuje pisanje ISKLJUCIVO na kraj fajla */
        flags |= O_CREAT;		/* flag kojim se fajl kreira u slucaju da ne postoji */
        break;
    default:
        return false;
    }
    
    /*
     * Open sistemskim pozivom se otvara fajl 
	 *
	 * TLPI za detalje
     */
    *fd = open(filePath, flags, defaultMode);
	/* provera gresaka */
    return *fd >= 0;
}


void os_create_hole(const char *pathname) {
    /*
     * Otvaramo fajl u modu za pisanje 
     */
    int fd;
    check_error(os_file_open(pathname, "w", &fd), "File open");

    /**
     * Pisemo neki tekst u fajl.
     */
    check_error(write(fd, "abc", 3) >= 0, "write");

    /*
     * Pomeramo offset iza kraja fajla.
     */
    off_t fileSize = lseek(fd, 10, SEEK_END);
    check_error(fileSize >= 0, "Lseek failed");

    /**
     * Ponovo pisemo neki tekst.
     */
    check_error(write(fd, "abc", 3) >= 0, "write");

    close(fd);
}

int main(int argc, char **argv) {
    check_error(2 == argc, "Argument missing");
    
    os_create_hole(argv[1]);

    /**
     * Ispisujemo ascii vrednosti karaktera u fajlu na standardni izlaz
     * 
     */
    FILE *ulaz = fopen(argv[1], "r");
    char c;

    while((c = fgetc(ulaz)) != EOF)
    {
      printf("%hhd ", c);
    }
    putchar('\n');

    fclose(ulaz);

    return 0;
}
