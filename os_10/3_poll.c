/*
Program demonstrira nacin za multipleksiranje fajlova putem 
sistemskog poziva poll. Kao argumenti komandne linije,
zadaju se putanje do fifo fajlova. 
Potrebno je sacekati na podatke u tim fajlovima, 
pri cemu se ne zna unapred kada ce i na kom fajlu pristici podaci. 
Poruke koje stizu potrebno je ispisati na standardni izlaz. Program
se zaustavlja kada neka od poruka bude "quit".
*/

#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <poll.h>
#include <strings.h>
#include <string.h>

#define check_error(cond,msg)\
  do {\
    if (!(cond)) {\
      perror(msg);\
      fprintf(stderr, "File: %s\nFunction: %s\nLine: %d\n", __FILE__, __func__, __LINE__);\
      exit(EXIT_FAILURE);\
    }\
  } while (0)

#define MAX_SIZE (256) 


static const char *os_Usage = "./poll pathToFile1 pathToFile2 ....";

int main(int argc, char** argv) {
	
	check_error(argc >= 2, os_Usage);
	
	/* alokacija memorije za strukture koje cuvaju informacije
	 * o fajl deskriptorima i aktivnosti koje se monitorisu 
	 */ 
	unsigned int numFifos = argc - 1;
	/* S obzirom da se radi o strukturama, nije lose
	 * odmah ih inicijalizovati na 0, pa zato koristimo calloc
	 */ 
	struct pollfd* fileMonitors = calloc(numFifos, sizeof(struct pollfd));
	check_error(fileMonitors != NULL, "calloc failed");
	
	/* inicijalizujemo polja sttruktura */
	int i;
	for (i = 0; i < numFifos; i++) {
		/* pamtimo fajl deskriptor
		 * 
		 * BITNO:
		 * fajl deskriptor mora biti otvoren u neblokirajucem rezimu (O_NONBLOCK),
		 * inace ne postoji potreba za multipleksiranjem
		 */ 
		fileMonitors[i].fd = open(argv[i + 1], O_RDONLY | O_NONBLOCK);
		check_error(fileMonitors[i].fd != -1, "open failed");
		/* u polje events se postavljaju aktivnosti koje zelimo da monitorisemo
		 * u ovom slucaju codimo racuna o dalaznim informacijama
		 */ 
		fileMonitors[i].events = POLLIN;
	}

	char buf[MAX_SIZE]; /* bafer u koji ucitavamo reci iz fifo-a */
	memset(buf, 0, sizeof(buf)); /* inicijalno prazan */
	
	/* u petlji */
	do {
		/* pozivamo funkciju poll i cekamo da se pojavi neka aktivnost */
		int events = poll(fileMonitors, numFifos, -1);
		check_error(events != -1, "poll failed");
		bool breakLoops = false;
		
		/* prolazimo kroz ceo niz i okrivamo na kom fajl deskriptoru(ima)
		 * se desila aktivnost
		 */
		for (i = 0; i < numFifos; i++) {
			/* BITNO:
			 * obratiti paznju da se za proveru koristi polje revents
			 * u strukturi pollfd
			 */ 
			if (fileMonitors[i].revents & POLLIN) {
				/* citamo sadrzaj fifoa */
				int bytesRead = read(fileMonitors[i].fd, buf, MAX_SIZE);
				/* terminiramo bafer */
				buf[bytesRead] = 0;
				check_error(bytesRead != -1, "read failed");
				
				/* stampamo poruku */
				printf("FIFO %d send: %s\n", i, buf);
				
				/* BITNO:
				 * flag koji je postavljen mora rucno da se obrise 
				 */ 
				fileMonitors[i].revents = 0;
				
				/* ako je u bilo kom fifo uneto quit, postavljamo signal
				 * za prekid izvrsavanja
				 */ 
				if (!strncasecmp(buf, "quit", 4)) {
					breakLoops = true;
					break;
				}
			}
		}
		
		if (breakLoops)
			break;
			
	} while (true);
	
	/* zatvaramo fajl deskriptore */
	for (i = 0; i < numFifos; i++) {
		close(fileMonitors[i].fd);
	}
	
	/* oslobadjamo memoriju */
	free(fileMonitors);
	
	exit(EXIT_SUCCESS);
}
