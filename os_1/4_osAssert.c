/*
Primeri makroa za obradu gresaka koje cemo koristiti
na kursu. Ideja je za sitnije provere izbeci dodatne
funkcijske pozive.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
Marko checkError ispisuje poruku msg na standardni
izlaz za greske u slucaju neispunjenosti uslova cond.

checkError ne koristi funkcijski poziv, 
stoga su neophodne zagrade za cond (probati
bez zagrada).
*/
#define checkError(cond,msg)\
  do {\
    if (!(cond)) {\
      perror(msg);\
      fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);\
      exit(EXIT_FAILURE);\
    }\
  } while (0);


/*
Makro osAssert ispisuje poruku msg na standardni
izlaz za greske u slucaju neispunjenosti uslova cond,
kao i ime fajla i broj linije u kojoj je nastala greska.
Za razliku od checkError makroa, ovaj makro koristi funkcijski poziv.
*/
#define osAssert(cond, msg) osErrorFatal(cond, msg, __FILE__, __LINE__)

void osErrorFatal(bool cond, const char *msg, const char *file, int line)
{
   if (!cond) {
     perror(msg);
     fprintf(stderr, "%s:%d\n", file, line);
     exit(EXIT_FAILURE);
   }
}

int main(){
  FILE *ulaz= fopen("ulaz.txt","r");

  checkError(ulaz != NULL, "Otvaranje datoteke nije uspelo");
  //osAssert(ulaz != NULL, "Otvaranje datoteke nije uspelo");

  return 0;
}