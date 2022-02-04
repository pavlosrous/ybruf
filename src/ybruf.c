#include <netinet/in.h>
#include <signal.h>

//#include "/Users/Tien 1/Desktop/bravo-project/include/ybruf.h"
#include "ybruf.h"
//-----------------------------------ISSUES------------------------------------------------------
//When trying to connect to ports up to 1000, I would get "bind/listen: Permission denied"
//When I terminate with -USR1 and then try to reconect to the same port number when I relunch, I get "Already in use"


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
    perror("More than 2 arguments\n");
    return false;
  }

  else if (argn == 2)
  {
    int p2 = atoi(argv[1]); //coverting the second parameter into an integer
    if (p2 < 0)
    {
      fprintf(stderr, "Usage: %s [%s]\n", argv[0],argv[1]);
      return false;
    }
    else
    {
      PORT = p2;
    }
  }

  /* 2 */
  if (chdir(APP_WD) != 0) //returns -1 if it fails
  {
    perror(APP_WD);
    return false;
  }

  /* 3 */
  FILE *fptr;

  fptr = fopen("ybruf.pid", "w");

  if (fptr == NULL)
  {
    perror("File cannot be created or a case of a write error\n"); //need to make changes to the dir
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
  if (signo==SIGUSR1){
    done = true;
  }
  else{
    exit(EXIT_SUCCESS);
  }

  
#ifdef DEBUG
  fprintf(stderr, "Terminated by %d\n", signo);
#endif
}

// The main function
int main(int argn, char *argv[])
{

  bool status = app_initialize(argn, argv);
  if (status == false)
    return EXIT_FAILURE;

  // Create a socket
  struct sockaddr_in serv_addr, cli_addr;
  int sockfd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET since its internet socket, SOCK_STREAM to specify that we are using a TCP protocol and 0 also 
  // characterizes the TCP socket
  if (-1 == sockfd)
  {
    perror("socket");
    return EXIT_FAILURE;
  }

  // Bind the socket and start listening on it
  bzero((char *)&serv_addr, sizeof(serv_addr));//ask about this

  serv_addr.sin_family = AF_INET; //sets the family of the address (internet address)

  serv_addr.sin_addr.s_addr = INADDR_ANY; //server addres we will be connecting to. We use any address connected to our local machine

  serv_addr.sin_port = htons(PORT); //we specify the port we are connecting to. We need to use the htons() to put the integer port in the right
  // network byte order. htons() converts it into the right data format so that our struct understands the port number

  if (-1 == bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) || -1 == listen(sockfd, 5 /* backlog */))
  {//if we wanted to connected to a socket in another machine, we would use connect() instead. Here we create a server (not a client). We bind it to a 
  //socket number and IP. Listen function listens for connections
  //bind specifies from where our sockets listens for connections
  //if 0, everything is ok, if -1 something went wrong and return error message
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
    //when we listen to a connection, we accept it and we get back a socket to manipulate anything on the client side so we have a two way connection 
    //with the client and server
    if (-1 == newsockfd)
    { // What can we do? Cannot even print an error message
#ifdef DEBUG
      perror("accept");
#endif
      continue;
    }
    fputs("Hello, world!\n", stderr);
  }
  close(sockfd);
  return EXIT_SUCCESS;

}

