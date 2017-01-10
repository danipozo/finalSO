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

#define ERR_CHECKL0(code, syscall) sprintf(err_msg_str, "%s:%d %s", __FILE__, __LINE__, syscall); \
                                   error_check(code, is_error_l0, handle_error, err_msg_str);

void error_check(int code, bool(*is_error)(int), void(*on_error)(char*), char* msg)
{
  if(is_error(code))
    {
      on_error(msg);
    }
  #ifdef DEBUG
  else{
    static int n_checks = 1;

    printf("ERROR CHECK: Check %d succeeded\n", n_checks++);
  }
  #endif
}

/* Less-than-zero errors. */
bool is_error_l0(int code)
{
  return code < 0;
}

void handle_error(char* msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

void TERM_handler(int s)
{
  printf("SERVER: Termination signal received\n");
  exit(EXIT_SUCCESS);
}

int main()
{
    char err_msg_str[40];
    char buf[1024];

    struct sigaction sa;
    sa.sa_handler = TERM_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    int sret = sigaction(SIGTERM, &sa, NULL);

    mknod("fifoIn", S_IFIFO|S_IRWXU|S_IRWXG, 0);
    mknod("fifoOut", S_IFIFO|S_IRWXU|S_IRWXG, 0);

    int fd = open(".spool.lock", O_RDWR|O_CREAT, S_IRWXU|S_IRWXG);

    ERR_CHECKL0(fd, "open")

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

      int nclients = size/4;
      size -= nclients*4;

      for(int i=0; i<nclients; i++)
        {
          // Serve client.
        }

    }

    return 0;
}
