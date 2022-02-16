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
  if (argn > 2)
  {
    perror("More than 2 arguments");
    return false;
  }

  else if (argn == 2)
  {
    int p2 = atoi(argv[1]); //coverting the second parameter into an integer
    if (p2 < 0)
    {
      fprintf(stderr, "Second parameter is a negative number");
      return false;
    }
    else
    {
      PORT = p2;
    }
  }
  /* 2 */
  if (chdir(APP_WD) != 0)
  {
    perror("chdir() to APP_WD failed");
    return false;
  }

  /* 3 */
  FILE *fptr;

  fptr = fopen("ybruf.pid", "w");

  if (fptr == NULL)
  {
    perror("File cannot be created or a case of a write error");
    return false;
  }
  
  int server_ID = getpid();
  fprintf(fptr, "%d", server_ID);
  fclose(fptr);

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
  if (-1 == sockfd)
  {
    perror("socket");
    return EXIT_FAILURE;
  }

  // Bind the socket and start listening on it
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);
  if (-1 == bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) || -1 == listen(sockfd, 5 /* backlog */))
  {
    perror("bind/listen");
    return EXIT_FAILURE;
  }

  signal(SIGUSR1, app_terminate);
  signal(SIGUSR2, app_terminate);

  // The main loop
  while (!done)
  {
    unsigned clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (-1 == newsockfd)
    { // What can we do? Cannot even print an error message
#ifdef DEBUG
      perror("accept");
#endif
      continue;
    }
    fputs("Hello, world!\n", stderr);
  }
  return EXIT_SUCCESS;
}
