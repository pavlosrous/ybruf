#include <netinet/in.h>
#include <signal.h>

#include "/home/prousoglou/Desktop/Team-bravo/bravo-project/include/ybruf.h"

// Global parameters
const int MIN_SERVERS = 3, MAX_SERVERS = 10;

// Port for the server
static int PORT = 8000;

// Are we done yet?
static bool done = false;

static bool app_initialize(int argn, char *argv[])
{
  /* Check usage */
  // More than one arg OR one arg AND not a positive number?
  if ((argn > 2) || ((argn == 2) && ((PORT = atoi(argv[1])) <= 0))) {
    fprintf(stderr, "USAGE: %s [port-number]\n", argv[0]);
    return false;
  }

  /* Change working directory */
  if (-1 == chdir(APP_WD)) {
    perror(APP_WD);
    return false;
  }
  
  /* Save the process ID */
  FILE *pidfile;
  if (!(pidfile = fopen(APP_PIDFILE, "w"))) {
    perror(APP_PIDFILE);
    return false;
  } else {
    fprintf(pidfile, "%d", getpid());
    fclose(pidfile);
  }

  /* Log the initialization message */
  syslog(LOG_INFO, "Started on port %d", PORT);

  return true;
}

/* 
 * Terminate the main loop 
 * kill -USR1 `cat /tmp/app/app.pid`
 */
static void app_terminate(int signo)
{
  // Log the termination signal
  syslog(LOG_INFO, "Terminated by signal %d", signo);

  switch (signo) {
  case SIGUSR1:			// "Soft" termination
    done = true;
    break;
  case SIGUSR2:			// "Hard" termination
    exit(EXIT_SUCCESS);
  default:			// Do nothing
    break;
  }
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
    syslog(LOG_ERR, "socket(): %s", strerror(errno));
    return EXIT_FAILURE;
  }

  // Bind the socket and start listening on it
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);
  if (   -1 == bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
      || -1 == listen(sockfd, 5 /* backlog */)) {
    syslog(LOG_ERR, "bind()/listen(): %s", strerror(errno));
    return EXIT_FAILURE;
  }

  // Register the signal handler(s)
  signal(SIGUSR1, app_terminate);
  signal(SIGUSR2, app_terminate);

  // The main loop
  while (!done) {
    unsigned clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (-1 == newsockfd) { // Scrap it
      syslog(LOG_ERR, "accept(): %s", strerror(errno));
      continue;
    }

    // Fetch and process the request
    process_request(newsockfd);
  }
  return EXIT_SUCCESS;
}

