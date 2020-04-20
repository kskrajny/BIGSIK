#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "err.h"

#define BUFFSIZE 4096 //duze ze wzgledu na cookies

int main(int argc, char *argv[])
{

  /* Kontrola dokumentów ... */
  if (argc != 3)
  {
    syserr("Usage: %s cookies addr_to_test", argv[0]);
  }

  char buff[BUFFSIZE];
  char buf[BUFFSIZE];
  char *addr = argv[2];
  char *prog;
  /* Rozpoznanie */
  if (strstr(addr, "https") != NULL)
  {
    /* Użyję trzech procesów, które kolejno:
      - otworzą tunel
      - wykonają skrypt testhttp_raw
      - zamkną tunel
      Te czynności będą odbywać się sekwencyjnie
    */

    pid_t pid;
    switch (pid = fork())
    {

    case -1:
      syserr("Error in fork\n");

    case 0:
      memset(buff, 0, BUFFSIZE);
      char *cwd = getcwd(buff, BUFFSIZE);
      memset(buf, 0, BUFFSIZE);
      addr += 8;
      addr = strsep(&addr, "/");
      if (snprintf(buf, BUFFSIZE, "pid = %s/pid\n\
[service]\nclient = yes\naccept = 127.0.0.1:3333\n\
connect = %s:443", cwd, addr) < 0)
        syserr("snprintf");
      FILE *fp;
      fp = fopen("stu", "w");
      if (fp == NULL)
        syserr("fopen");
      fwrite(buf, strlen(buf), 1, fp);
      if (ferror(fp) != 0)
        syserr("fwrite");
      if (fclose(fp) != 0)
        syserr("fclose");
      prog = "stunnel";
      char *arg = "stu";
      if (execlp(prog, prog, arg, NULL) == -1)
        syserr("execlp");

    default:

      if (wait(0) == -1)
        syserr("wait\n");
      remove("stu");

      switch (pid = fork())
      {

      case -1:
        syserr("Error in fork\n");

      case 0:
        prog = "./testhttp_raw";
        addr = "127.0.0.1:3333";

        if (snprintf(buff, BUFFSIZE, "%s", addr) < 0)
          syserr("snprintf");

        if (execlp(prog, prog, buff, argv[1], argv[2], NULL) == -1)
          syserr("execlp");

      default:
        if (wait(0) == -1)
          syserr("wait\n");
        prog = "kill";
        FILE *fp;
        fp = fopen("pid", "r");
        if (fp == NULL)
          syserr("fopen");
        memset(buf, 0, BUFFSIZE);
        fread(buf, BUFFSIZE, 1, fp);
        if (snprintf(buff, strlen(buf), "%s", buf) < 0)
          syserr("snprintf");
        if (ferror(fp) != 0)
          syserr("fread");
        if (fclose(fp) != 0)
          syserr("fclose");
        remove("pid");
        if (execlp(prog, prog, buff, NULL) == -1)
          syserr("execlp");
      }
    }
  }
  else
  {
    char *prog = "./testhttp_raw";
    strcpy(buf, argv[2]);
    addr += 7;
    addr = strsep(&addr, "/");
    char *help = addr;
    help = strstr(help, ":");
    if(help == NULL) { 
      if (snprintf(buff, BUFFSIZE, "%s:80", addr) < 0)
        syserr("snprintf");
    } else {
      if (snprintf(buff, BUFFSIZE, "%s", addr) < 0)
        syserr("snprintf");
    }
    if (execlp(prog, prog, buff, argv[1], buf, NULL) == -1)
      syserr("execlp");
  }

  return 0;
}
