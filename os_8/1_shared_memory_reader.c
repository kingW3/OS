/*
Poziv programa:
./shared_memory_reader pathToFile

Program prvo ucitava objekat deljene memorije na putanji 'pathToFIle'
a potom ispisuje niz brojeva iz tog objekta.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/mman.h>

#define osErrorFatal(userMsg) osErrorFatalImpl((userMsg), __FILE__, __func__, __LINE__)
#define osAssert(expr, userMsg) \
	do {\
		if (!(expr)) \
			osErrorFatal(userMsg); \
	} while (0)
	
void osErrorFatalImpl(const char* userMsg, const char* fileName, const char* functionName, const int lineNumber);

static char* osUsage = "./shared memory reader filePath";

void *osGetMemoryBlock(const char* filePath, unsigned* size);

int main(int argc, char** argv) {
	
	osAssert(argc == 2, osUsage);
	
	/* ucitavamo parce deljene memorije
	 * BITNO:
	 * ovde samo ucitavamo sadrzaj objekta deljene memorije,
	 * ne kreiramo ga. Kreiranje radi samo jedan proces
	 */ 
	unsigned size = 0;
	int *niz = osGetMemoryBlock(argv[1], &size);
	int brojEl = size/sizeof(int);
	
	/* citamo sadrzaj niza iz mapiranog bloka */
	int i = 0;
	for (i = 0; i < brojEl; i++)
		printf("%d ", niz[i]);
		
	printf("\n");
	
	/* izmene memorijsko bloka se trajno snimaju na disku, tj.
	 * u fajlu koji predstavlja deljenu memoriju 
	 */
	osAssert(munmap(niz, size) != -1, "munmap failed");
	/* brisemo objekat deljene memorije, jer nam vise ne treba 
	 * unlink radi poslednji proces koji koristi objekat deljene memorije
	 */ 
	osAssert(shm_unlink(argv[1]) != -1, "shm_unlink failed");
	
	exit(EXIT_SUCCESS);
}

void *osGetMemoryBlock(const char* filePath, unsigned* size) {
	
	/* otvara se fajl koji predstavlja deljenu memoriju */
	int memFd = shm_open(filePath, O_RDONLY, 0);
	osAssert(memFd != -1, "shm_open failed");
	
	/* uz pomoc fstata se dobija velicina fajla
	 * 
	 * BITNO:
	 * nikada ne koristiti obican stat, vec uvek fstat
	 * 
	 * JOS BITNIJE:
	 * samo jedan proces radi ftruncate i podesava velicinu fajla. SVI
	 * ostali pomocu fstat otkrivaju tu velicinu
	 */ 
	struct stat fInfo;
	osAssert(fstat(memFd, &fInfo) != -1, "fstat failed");
	*size = fInfo.st_size;
	
	void* addr;
	
	/* mapiranje objekta deljene memorije u RAM memoriju
	 * BITNO:
	 * prava pristupa memorijski mapiranom delu moraju da budu jednaka
	 * modu u kome je fajl deljene memorije otvoren
	 */ 
	osAssert((addr = mmap(0, *size, PROT_READ, MAP_SHARED, memFd, 0)) != MAP_FAILED, "mmap failed");
	
	close(memFd);
	
	return addr;
}

void osErrorFatalImpl(const char* userMsg, const char* fileName, const char* functionName, const int lineNumber) {
	
	perror(userMsg);
	
	fprintf(stderr, "File name: %s\nFunction name: %s\nLine number: %d\n", fileName, functionName, lineNumber);
	
	exit(EXIT_FAILURE);
}
