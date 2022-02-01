#include <netinet/in.h>
#include <signal.h>

#include "ybruf.h"

// Global parameters
const int MIN_SERVERS = 3, MAX_SERVERS = 10;

// Port for the server
static int PORT = 8000;

// Are we done yet?
static bool done = false;

static bool app_initialize(int argn, char *argv[])
{
  /* 1 */
  
  /* 2 */
  
  /* 3 */

  /* 4 */
  return true;
}

/* 
 * Terminate the main loop 
 * kill -USR1 `cat /tmp/app/app.pid`
 */
static void app_terminate(int signo)
{
#ifdef DEBUG
  fprintf(stderr, "Terminated by %d\n", signo);
#endif
  
}

// The main function 
int main(int argn, char *argv[])
{
  // Inilialize the process
  bool status = app_initialize(argn, argv);
  if (status == false)
    return EXIT_FAILURE;

  // Create a socket
  struct sockaddr_in serv_addr, cli_addr; 
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sockfd) {
    perror("socket");
    return EXIT_FAILURE;
  }

  // Bind the socket and start listening on it
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);
  if (   -1 == bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
      || -1 == listen(sockfd, 5 /* backlog */)) {
    perror("bind/listen");
    return EXIT_FAILURE;
  }
  
  signal(SIGUSR1, app_terminate);
  signal(SIGUSR2, app_terminate);

  // The main loop
  while (!done) {
    unsigned clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (-1 == newsockfd) { // What can we do? Cannot even print an error message
      #ifdef DEBUG
      perror("accept");
      #endif
      continue;
    }
    fputs("Hello, world!\n", stderr);
  }
  return EXIT_SUCCESS;
}

