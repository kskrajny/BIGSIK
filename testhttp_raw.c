#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "err.h"

#define BUFF_SIZE 4096 //duze ze wzgledu na cookies

int main(int argc, char *argv[])
{
  int rc;
  int sock;
  struct addrinfo addr_hints, *addr_result;
  char buff[BUFF_SIZE];
  unsigned long all = 0;
  int count = 0;
  /* Kontrola dokumentów ... */
  if (argc != 4)
  {
    syserr("Usage: %s host:port cookies addr_to_test", argv[0]);
  }

  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0)
    syserr("socket");

  /* Trzeba się dowiedzieć o adres internetowy serwera. */
  memset(&addr_hints, 0, sizeof(struct addrinfo));
  addr_hints.ai_flags = 0;
  addr_hints.ai_family = AF_INET;
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = IPPROTO_TCP;

  char *addr = strsep(&argv[1], ":");
  char *port = strsep(&argv[1], ":");
  char req[256];

  memset(req, 0, sizeof(req));

  if (snprintf(req, 256, "GET %s HTTP/1.1\r\nHost: \
%s\r\nConnection: closed\r\n", argv[3], addr) < 0)
    syserr("snprintf");

  FILE *rf;
  rf = fopen(argv[2], "r");
  if (rf == NULL)
    syserr("fopen");

  while (fgets(buff, BUFF_SIZE, rf) != NULL)
  {
    strncat(req, "Cookie: ", 9);
    strncat(req, buff, strlen(buff));
    strncat(req, "\r\n", 3);
  }

  strncat(req, "\r\n", 3);

  rc = getaddrinfo(addr, port, &addr_hints, &addr_result);
  if (rc != 0)
  {
    syserr("getaddrinfo: %s", gai_strerror(rc));
  }

  if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) != 0)
  {
    syserr("connect");
  }

  if (write(sock, (const void *)req, strlen(req)) < 0)
  {
    syserr("write");
  }

  memset(buff, 0, BUFF_SIZE);

  FILE *fd = fdopen(sock, "r");

  if (fgets(buff, BUFF_SIZE, fd) == NULL)
    syserr("fgets");
  char *res = buff;

  if (strstr(res, "200 OK") == NULL)
  {
    char *status = strsep(&res, "\n");
    printf("%s\n", status);
  }
  else
  {
    while (fgets(buff, BUFF_SIZE, fd) != NULL) {
      res = buff;
      if (strlen(res) == 0)
        break;
      if(strstr(res, "\r\n") == NULL && count == 1) {
        all += strlen(res);
      }
      if (strlen(res) == 2)
        count = 1;
      if (strstr(res, "Set-Cookie:") != NULL)
      {
        strsep(&res, " ");
        char *cookie = strsep(&res, ";");
        printf("%s\n", cookie);
      }
    }
    printf("Dlugosc zasobu: %lu\n", all);
  }

  freeaddrinfo(addr_result);

  if (fclose(fd) != 0)
  {
    syserr("fclose and stream close");
  }

  if (fclose(rf) != 0)
  {
    syserr("fclose");
  }

  return 0;
}
