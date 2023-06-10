/*
Primer ceste greske na ispitu. Kada u makrou neposredno nakon
karaktera '\' ne stavimo novi red (linija 16), moze doci
do nepravilnog ponistavanja novog reda. Takodje,
ukoliko se u potpunosti zaboravi karakter '\ 
dodje do greske koju nekad nije lako otkriti.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define checkError(cond,msg)\
  do {\
    if (!(cond)) {\
      perror(msg);\
      fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);\ 
      exit(EXIT_FAILURE);\
    }\
  } while (0);

int main(){
  FILE *ulaz= fopen("ulaz.txt","r");

  checkError(ulaz != NULL, "Otvaranje datoteke nije uspelo");
  //osAssert(ulaz != NULL, "Otvaranje datoteke nije uspelo");

  return 0;
}