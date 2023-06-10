/*
Program demonstrira citanje flegova za fajl i dodavanje novih.
*/

#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <sys/time.h>

#include <string.h>

#define check_error(expr,userMsg)\
	do {\
		if (!(expr)) {\
			perror(userMsg);\
			exit(EXIT_FAILURE);\
		}\
	} while(0)
// ./1 path .c
int main(int argc, char** argv) {
	
	/* citamo flagove za stdin */
	int stdinFlags = fcntl(STDIN_FILENO, F_GETFL);
	check_error(stdinFlags != -1, "...");
	
	/* iz spiska flagova izdvajamo mod u kome je fajl otvoren */
	int accessMode = stdinFlags & O_ACCMODE;
	char* message = "Mogu da pisem\n";
	
	/* proveravamo da li mozemo da pisemo po fajlu */
	if (accessMode == O_WRONLY || accessMode == O_RDWR) {
		
		/* i ako mozemo, pisemo */
		check_error(write(STDIN_FILENO, message, strlen(message)) != -1, "....");
	}
	
	/* otvaramo fajl u w modu */
	int fd = open("1.txt", O_WRONLY);
	check_error(fd != -1, "...");
	
	/* upisujemo sadrzaj u fajl */
	check_error(write(fd, message, strlen(message)) != -1, "....");
	
	/* citamo flagove fajla */
	int flags = fcntl(fd, F_GETFL);
	check_error(flags != -1, "...");
	
	/* dodajemo O_APPEND flag */
	flags |= O_APPEND;
	check_error(fcntl(fd, F_SETFL, flags) != -1, "....");
	
	/* upisujemo sadrzaj u fajl */
	check_error(write(fd, message, strlen(message)) != -1, "....");
	
	close(fd);
	
	exit(EXIT_SUCCESS);
}
