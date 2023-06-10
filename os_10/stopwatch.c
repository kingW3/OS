/*
Demonstrativni primer trajanja izvrsavanja u korisnickom i sistemskom modu.
(nije neophodan za ispit :))
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#define osErrorFatal(userMsg) osErrorFatalImpl((userMsg), __FILE__, __func__, __LINE__)
#define osAssert(expr, userMsg) \
    do { \
        if (!(expr)) \
            osErrorFatal(userMsg); \
    } while(0)
    
#define MAX_SIZE (4096)    

typedef struct timeval osTimePoint;

typedef struct {
	
	osTimePoint wallTime; /* ukupno vreme izvrsavanja: user + sys + block */
	osTimePoint userTime; /* vreme provedeno u korisnickom rezimu */
	osTimePoint sysTime;  /* vreme provedeno u sistemskom rezimu */ 
} osStopwatch;

void osErrorFatalImpl(const char *userMsg, const char *fileName, 
                      const char *functionName, const int lineNum);

/* funkcija oduzima dva vremena u rezultat upisuje u levi argument */
bool osSubtractTime(osTimePoint* left, osTimePoint* right) {
	
	size_t leftInUsec = left->tv_sec*1000000 + left->tv_usec;
	size_t rightInUsec = right->tv_sec*1000000 + right->tv_usec;
	
	if (rightInUsec > leftInUsec)
		return false;
		
	size_t timeDiff = leftInUsec - rightInUsec;
	left->tv_sec = timeDiff / 1000000;
	left->tv_usec = timeDiff % 1000000;
	
	return true;
}
    
 /* funkcija startuje merenje vremena */
void osStopwatchStart(osStopwatch *pStopwatch) {

	struct rusage rUsageStruct;
	/* getrusage daje sys i user vreme procesa */
	osAssert(getrusage(RUSAGE_SELF, &rUsageStruct) != -1, "getrusage failed");
	/* citamo wall vreme */
	osAssert(gettimeofday(&(pStopwatch->wallTime), NULL) != -1, "gettimeof day failed");
	
	/* pamtimo vremena
	 * 
	 * BITNO:
	 * kopiramo strukture pa je memcpy najzgodniji 
	 */ 
	memcpy(&(pStopwatch->userTime), &rUsageStruct.ru_utime, sizeof(osTimePoint));
	memcpy(&(pStopwatch->sysTime), &rUsageStruct.ru_stime, sizeof(osTimePoint));
}    

/* funkcija zaustavlja merenje vremena */
void osStopwatchStop(osStopwatch *pStopwatch) {
	
	struct rusage rUsageStruct;
	osTimePoint walltime;
	
	/* cita se trenutno vreme */
	osAssert(getrusage(RUSAGE_SELF, &rUsageStruct) != -1, "getrusage failed");
	osAssert(gettimeofday(&walltime, NULL) != -1, "gettimeofday failed");
	
	/* racuna se proteklo vreme od pocetka merenja */
	osAssert(osSubtractTime(&walltime, &(pStopwatch->wallTime)) != false, "time fix failed");
	osAssert(osSubtractTime(&rUsageStruct.ru_utime, &(pStopwatch->userTime)) != false, "time fix failed");
	osAssert(osSubtractTime(&rUsageStruct.ru_stime, &(pStopwatch->sysTime)) != false, "time fix failed");
	
	/* rezultati se upisuju u odgovarajuce promenljive */
	memcpy(&(pStopwatch->wallTime), &walltime, sizeof(osTimePoint));
	memcpy(&(pStopwatch->userTime), &rUsageStruct.ru_utime, sizeof(osTimePoint));
	memcpy(&(pStopwatch->sysTime), &rUsageStruct.ru_stime, sizeof(osTimePoint));
}    

/* funkcije vrse skaliranje na izabrane vrednosti */
double osStopWatch_WallTimeUSeconds(osStopwatch* p) {
	return (double)(p->wallTime.tv_sec*(size_t)1000000 + p->wallTime.tv_usec);
}

double osStopWatch_UserTimeUSeconds(osStopwatch* p) {
	return (double)(p->userTime.tv_sec*(size_t)1000000 + p->userTime.tv_usec);
}

double osStopWatch_SysTimeUSeconds(osStopwatch* p) {
	return (double)(p->sysTime.tv_sec*(size_t)1000000 + p->sysTime.tv_usec);
}

double osStopWatch_WallTimeSeconds(osStopwatch* p) {
	return (double)(p->wallTime.tv_sec*(size_t)1000000 + p->wallTime.tv_usec)/1000000.0;
}

double osStopWatch_UserTimeSeconds(osStopwatch* p) {
	return (double)(p->userTime.tv_sec*(size_t)1000000 + p->userTime.tv_usec)/1000000.0;
}

double osStopWatch_SysTimeSeconds(osStopwatch* p) {
	return (double)(p->sysTime.tv_sec*(size_t)1000000 + p->sysTime.tv_usec)/1000000.0;
}

uint64_t osStopWatch_WallTimeUSecondsUint(osStopwatch* p) {
	return (uint64_t)(p->wallTime.tv_sec*(uint64_t)1000000 + p->wallTime.tv_usec);
}

uint64_t osStopWatch_UserTimeUSecondsUint(osStopwatch* p) {
	return (uint64_t)(p->userTime.tv_sec*(uint64_t)1000000 + p->userTime.tv_usec);
}

uint64_t osStopWatch_SysTimeUSecondsUint(osStopwatch* p) {
	return (uint64_t)(p->sysTime.tv_sec*(uint64_t)1000000 + p->sysTime.tv_usec);
}
   
static const char *os_Usage = "";

int main(int argc, char** argv) {
	
	osStopwatch st;
	char buf[MAX_SIZE];
	int i;
	
	int fd = open("/dev/urandom",O_RDONLY);
	osAssert(fd != -1, "open failed");
	printf("Kernelsko vreme...\n");
	osStopwatchStart(&st);
	for (i = 0; i < 5000; i++)
		osAssert(read(fd, buf, MAX_SIZE) != -1, "read failed");
	osStopwatchStop(&st);
	printf("Ukupno izmereno:\nSys: %.2lfs\nUser: %.2lfs\nWall: %.2lfs\n",\
			osStopWatch_SysTimeSeconds(&st), \
			osStopWatch_UserTimeSeconds(&st), \
			osStopWatch_WallTimeSeconds(&st));
	
	printf("Korisnicko vreme...\n");
	osStopwatchStart(&st);
	int j = 0;
	for (i = 0; i < 5000000; i++)
		j = j/(i+1)/(10)*(50*(i+1));
	osStopwatchStop(&st);
	printf("Ukupno izmereno:\nSys: %.2lfs\nUser: %.2lfs\nWall: %.2lfs\n",\
			osStopWatch_SysTimeSeconds(&st), \
			osStopWatch_UserTimeSeconds(&st), \
			osStopWatch_WallTimeSeconds(&st));
	
	printf("Korisnicko vreme...\n");
	osStopwatchStart(&st);
	sleep(3);
	osStopwatchStop(&st);
	printf("Ukupno izmereno:\nSys: %.2lfs\nUser: %.2lfs\nWall: %.2lfs\n",\
			osStopWatch_SysTimeSeconds(&st), \
			osStopWatch_UserTimeSeconds(&st), \
			osStopWatch_WallTimeSeconds(&st));
	
	exit(EXIT_SUCCESS);
}

void osErrorFatalImpl(const char *userMsg, const char *fileName, 
                      const char *functionName, const int lineNum) {
    perror(userMsg);
    fprintf(stderr, "File: '%s'\nFunction: '%s'\nLine: '%d'\n", fileName, functionName, lineNum);
    exit(EXIT_FAILURE);
}
