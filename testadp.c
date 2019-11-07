#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define FD_UNINIT_C -1

#define SERVER_PORT 2019
#define LISTEN_QUEUE 1

#define MAX_SENDBUF_LEN 80

#define ERR_SOCK -1
#define SUCCESS 0
#define EC_NOBUFF 200

#define TRUE 1
#define FALSE 0

#define CMG_DBG 1
#if CMG_DBG
#define TEST_LOOPS 100
#define CMG_PRINT(x) printf x
#else
#define CMG_PRINT(x) do {} while (0)
#endif

void dieWithError(char *dieWithError_msg)
{
  printf("Error: %s\n", dieWithError_msg);
  exit(1);
}

int mtprint_time_message(char *buf) {
  int len = -1;
  time_t t = time(NULL);

  if (buf != NULL) {
    snprintf(buf, MAX_SENDBUF_LEN, "|message|%s\n", asctime(localtime(&t)));
    len = strlen(buf);
  }

  return len;
}

void main_loop()
{
  // Define locals
  char close_server = 0;
  char client_connected = 0;
  int sock_so_reuseaddr = 1;
  int listenfd = FD_UNINIT_C;
  int clientfd = FD_UNINIT_C;
  struct sockaddr_in serveraddr;
  struct sockaddr_in clientaddr;
  socklen_t clientaddr_size;
  char sendbuf[MAX_SENDBUF_LEN];
  int sendbuf_len = 0;

  // Setup server
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == ERR_SOCK) {
    dieWithError("Could not allocate socket");
  }

  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sock_so_reuseaddr,
      sizeof(sock_so_reuseaddr)) == ERR_SOCK) {
    dieWithError("Could not set server socket options\n");
  }

  CMG_PRINT(("[CMG] Have a server socket\n"));

  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(SERVER_PORT);
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == ERR_SOCK) {
    dieWithError("Could not bind server socket");
  }

  CMG_PRINT(("[CMG] Bound\n"));

  if (listen(listenfd, LISTEN_QUEUE) == ERR_SOCK) {
    dieWithError("Could not listen");
  }

  CMG_PRINT(("[CMG] Listening\n"));

  // Loop while not servicing a client
  while (!close_server) {

    // Find a new client
    clientaddr_size = sizeof(clientaddr);
    memset(&clientaddr, 0, clientaddr_size);
    clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddr_size);
    if (clientfd == ERR_SOCK) {
      dieWithError("Error accepting connection");
    }

    client_connected = TRUE;

    CMG_PRINT(("Accepted a client connection\n"));
  
    // Loop while client is connected
    while(client_connected) {
      int status = SUCCESS;
      int idx;
      CMG_PRINT(("Client has connected, sleep!\n"));
      for (idx = 0; idx < TEST_LOOPS; idx++) {
        CMG_PRINT(("Sending data..\n"));
        memset(&sendbuf, 0, sizeof(sendbuf));
        sendbuf_len = mtprint_time_message(sendbuf);
        CMG_PRINT(("%s", sendbuf));
        status = send(clientfd, sendbuf, sendbuf_len, 0);
#if 0
        status = send(clientfd, "|message|CHG_INSRT|Change Inserts\n", 34, 0);
#endif
        if (status <= ERR_SOCK) {
          CMG_PRINT((" ERROR SENDING:%x\n", errno));
          break;
        } else {
          CMG_PRINT(("Wrote %#x bytes\n", status));
        }
        sleep(1);
      }
      client_connected = FALSE;
    }

    CMG_PRINT(("Done with client, cleaning up\n"));

    close(clientfd);
    close_server = TRUE;
  }

  close(listenfd);

  CMG_PRINT(("Done with server, exiting\n"));

  return;
}

int main(int argc, char *argv[])
{
  puts("hello christpoher!");
  printf("hey: %s\n", argv[0]);
  main_loop();
  puts("DONE..");
  return SUCCESS;
}
