#ifndef ERROR_CHECK_H
#define ERROR_CHECK_H

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

static char err_msg_str[40];

#define ERR_CHECK_F(code, syscall, f) sprintf(err_msg_str, "%s:%d %s", __FILE__, __LINE__, syscall); \
  error_check(code, f, handle_error, err_msg_str);

#define ERR_CHECKL0(code, syscall) ERR_CHECK_F(code, syscall, is_error_l0)
#define ERR_CHECKEQ0(code, syscall) ERR_CHECK_F(code, syscall, is_error_eq0)

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

bool is_error_eq0(int code)
{
  return code == 0;
}

void handle_error(char* msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

#endif // ERROR_CHECK_H
