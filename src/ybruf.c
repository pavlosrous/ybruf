#include <netinet/in.h>
#include <syslog.h>
#include <signal.h>

#include "ybruf.h"

// Global parameters
const int MIN_SERVERS = 3, MAX_SERVERS = 10;
const int MAX_RQ_SIZE = 1048576; // 1MB

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

  syslog(LOG_INFO,"Started on port %d", PORT);

  return true;
}

/* 
 * Terminate the main loop 
 * kill -USR1 `cat /tmp/app/app.pid`
 */
static void app_terminate(int signo)
{
  syslog(LOG_INFO,"Terminated by the signal %d", signo);

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

// Request processor
bool process_request(int sock_id)
{
  char buff[MAX_RQ_SIZE];
  char* header = "HTTP/1.1 200 OK\r\n\r\n";

  int read_status = read(sock_id,buff,MAX_RQ_SIZE);

  if (read_status == -1){
    syslog(LOG_ERR,"Reading buffer: %s",strerror(errno));
    return false;
  }


  int write_header = write(sock_id, header, strlen(header));
  int write_cliRequest = write(sock_id,buff,read_status); 

  if (write_cliRequest == -1){
    syslog(LOG_ERR,"Writing header in socket: %s",strerror(errno));
  }

  if (write_header == -1){
    syslog(LOG_ERR,"Writing message in socket: %s",strerror(errno));
  }

  int close_status = close(sock_id);
  if (close_status == -1){
    syslog(LOG_ERR,"Closing socket: %s",strerror(errno));
  }

  return true;  
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
    syslog(LOG_ERR,"Socket creation: %s",strerror(errno));
    return EXIT_FAILURE;
  }

  // Bind the socket and start listening on it
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);
  if (   -1 == bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
      || -1 == listen(sockfd, 5 /* backlog */)) {
    syslog(LOG_ERR,"bind/listen: %s", strerror(errno));
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
      syslog(LOG_ERR,"accept: %s",strerror(errno));
      continue;
    }

    process_request(newsockfd);
  }
  return EXIT_SUCCESS;
}