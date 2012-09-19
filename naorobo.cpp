// Programa besta que fica esperando o usuario apertar 'x' e quando
// isso acontece envia SIGUSR1 para o processo <click>
// Signal Handling
#include <stdio.h>     /* standard I/O functions                         */
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <signal.h>    /* signal name macros, and the signal() prototype */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  while(true) {
    while(getchar()) {
      // envia SIGUSR1 para o processo <click>
      system("killall -s SIGINT click");
      printf("Enviei!\n");
    }
  }
  return 0;
}
