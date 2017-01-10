#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>

#include "error_check.h"

void TERM_handler(int s)
{
  printf("SERVER: Termination signal received\n");
  exit(EXIT_SUCCESS);
}

void CHLD_handler(int s)
{
  wait(NULL);
}


int main()
{
    char buf[1024];

    struct sigaction sa;
    sa.sa_handler = TERM_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    int sret = sigaction(SIGTERM, &sa, NULL);
    ERR_CHECKL0(sret, "sigaction")

    sa.sa_handler = CHLD_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sret = sigaction(SIGCHLD, &sa, NULL);
    ERR_CHECKL0(sret, "sigaction")

    mknod("fifoIn", S_IFIFO|S_IRWXU|S_IRWXG, 0);
    mknod("fifoOut", S_IFIFO|S_IRWXU|S_IRWXG, 0);

    int lock_fd = open(".spool.lock", O_RDWR|O_CREAT, S_IRWXU|S_IRWXG);

    ERR_CHECKL0(lock_fd, "open")

    int fdFifoIn = open("fifoIn", O_RDWR);
    int fdFifoOut = open("fifoOut", O_RDWR);

    ERR_CHECKL0(fdFifoIn, "open")
    ERR_CHECKL0(fdFifoOut, "open")


    int size = 0;
    while(true)
    {
      int current_size = read(fdFifoIn, buf+size, 1024-size);
      ERR_CHECKL0(current_size, "read")

      size += current_size;

      int nclients = size/sizeof(int);
      size -= nclients*sizeof(int);

      for(int i=0; i<nclients; i++)
      {
        // Serve client.
        int pid = fork();
        ERR_CHECKL0(pid, "fork")

        if(!pid)
        {
          char str[4];
          sprintf(str, "%d", lock_fd);
          execlp("./proxy", "proxy", str, NULL);
        }
        else
        {
          write(fdFifoOut, &pid, sizeof(int));
        }
      }

    }

    return 0;
}
