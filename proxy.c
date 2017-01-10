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

int main(int argc, char** argv)
{
  char buf[1024];

  int lock_file_fd = atoi(argv[1]);

  char fifoName[20];
  sprintf(fifoName, "fifo.%d", getpid());

  int ret = mkfifo(fifoName, 0666);
  ERR_CHECKL0(ret, "mkfifo")

  ERR_CHECKL0(kill(getppid(), SIGUSR1), "kill")

  int fifoFd = open(fifoName, O_RDONLY);
  ERR_CHECKL0(fifoFd, "open");

  FILE* tmp = tmpfile();
  ERR_CHECKEQ0(tmp, "tmpfile")

  int size;
  while(size = read(fifoFd, buf, 1024))
  {
      ERR_CHECKL0(size, "read")

      fwrite(buf, 1, size, tmp);
  }

  close(fifoFd);
  unlink(fifoName);

  struct flock l;
  l.l_type = F_WRLCK;
  l.l_whence = SEEK_SET;
  l.l_start = 0;
  l.l_len = 0;

  ret = fcntl(lock_file_fd, F_SETLKW, &l);
  ERR_CHECKL0(ret, "fcntl:lock")

  fseek(tmp, 0, SEEK_SET);
  while(size = fread(buf, 1, 1024, tmp))
  {
    fwrite(buf, 1, size, stdout);
  }

  if(ferror(tmp))
  {
    printf("PROXY: An error occurred reading temporary file\n");
    exit(EXIT_FAILURE);
  }

  fclose(tmp);
  close(lock_file_fd);

  return 0;
}
