/*
Funkcije osCheckFIFOHasReadEnd i osCheckFIFOHasWriteEnd proveravaju da li
prosledjeni fifo fajl ima otvoren odgovarajuci kraj u slucaju neblokirajuceg I/O.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#define check_error(cond,msg)\
  do {\
    if (!(cond)) {\
      perror(msg);\
      fprintf(stderr, "File: %s\nFunction: %s\nLine: %d\n", __FILE__, __func__, __LINE__);\
      exit(EXIT_FAILURE);\
    }\
  } while (0)

static const char *os_Usage = "Usage: ./check_fifo path/to/fifo mode (r or w)";

bool osCheckFIFOHasWriteEnd(const char *fifoPath)
{
	/* otvaramo fifo u modu za citanje
	 * O_NONBLOCK oznacava neblokirajuci IO, tj. open ce se vratiti odmah
	 * poziva, bez cekanja da neki drugi proces otvori proces u modu za citanje
	 * u slucaju da niko ne postoji na drugoj strani 
	 */
    int fifoFd = open(fifoPath, O_RDONLY | O_NONBLOCK);
    check_error(-1 != fifoFd, "Opening FIFO for reading failed");
    
	/* da bismo proverili da li zaista postoji neko na drugom kraju fifoa
	 * potrebno je da ucitamo makar jedan bajt
	 */
    char c;
    bool retVal;
    int bytesRead = read(fifoFd, &c, 1);
	/* u slucaju da read vrati 0, to znaci da ne postoji proces
	 * koji je otvorio fifo u modu za pisanje
	 */
    if (0 == bytesRead) {
	  /* zato povratnu vrednost postavljamo na false */
      retVal = false;
    } else if (bytesRead < 0) {
	  /* ukoliko read vrati -1, postoji dva slucaja
	   * 	1. da imamo proces na drugom kraju, ali da nemamo podataka
	   * 	2. da se dogodila greska prilikom citanja
	   * 
	   * Da bismo to proverili moramo da ispitamo vrednost errno-a
	   */
	   
	  /* ako je errno postvaljen na EAGAIN, to znaci da imamo proces na drugom kraju
	   * ali da nemamo podataka u fifo
	   */
      if (EAGAIN == errno) {
		/* pa povratnu vrednost postavljamo na true */
        retVal = true;
      } else {
		/* ako se radi o nekoj drugoj vrednosti errno-a, znaci da se dogodila neka
		 * greska i zato prekidamo izvrsavanje programa
		 */
        check_error(false, "Read failed when checking if FIFO has writter");
      }
    } else {
	  /* u slucaju da read vrati vrednost vecu od nula, znaci da imamo proces
	   * na drugoj strani fifoa koji je upisao neke podatke u fifo
	   */
      retVal = true;
    }
    
	/* zatvaramo fajl deskriptor */
    close(fifoFd);
    return retVal;
}

bool osCheckFIFOHasReadEnd(const char *fifoPath)
{	
	/* otvaramo fajl deskriptor u neblokirajucem modu za pisanje
	 *
	 * open poziv se odmah vraca i potrebno je da ispitamo vrednost errno 
	 */
    int fifoFd = open(fifoPath, O_WRONLY | O_NONBLOCK);
	
	/* u slucaju da je open pukao, neophodno je ispitati vrednost errnoa */
    if (-1 == fifoFd) {
		/* ako je vrednost errno-a ENXIO to znaci da na drugom kraju nemamo
		 * proces koji je otvorio fifo u modu za citanje 
		 */
        if (errno == ENXIO) {
			/* zato vracamo false */
            return false;
        }
		/* bilo koja druga vrednost errno-a znaci da se dogodila greska prilikom
		 * otvaranja fajla
		 */
        check_error(0, "Failed to open FIFO for writing");
    }
	
	/* zatvaramo fajl deskriptor */
    close(fifoFd);
	/* i vracamo true, jer je open prosao bez greske */
    return true;
}

int main(int argc, char **argv) {
    check_error(3 == argc, os_Usage);
    
	/* u zavisnosti od korisnikovog unosa ispitujemo krajeve fifo-a */
    bool res = false;
    if ('r' == argv[2][0]) {
        res = osCheckFIFOHasReadEnd(argv[1]);
    } else if ('w' == argv[2][0]) {
        res = osCheckFIFOHasWriteEnd(argv[1]);
    } else {
        check_error(0, "Wrong checking mode (see usage)");
    }
    
	/* stampamo rezultat ispitivanja */
    printf("%s\n", res ? "true" : "false");
	
    return 0;
}


