#include <fcntl.h>

#include "ybruf.h"

// Memory configuration parameters
static const int MAX_RQ_SIZE = 1048576; // 1MB
static const int MAX_DATA_SIZE = 1048576; // 1MB

bool write_http_header(int sock_id, const char *status, const char *msg)
{
  // YOUR CODE HERE
  
  return true;
}

bool process_GET(int sock_id, char *doc)
{
  char data[MAX_DATA_SIZE];

  /* Update the doc name */
  // YOUR CODE HERE

  /* Copy the file into the socket, if possible */
  // YOUR CODE HERE

  return true;
}

bool process_request(int sock_id)
{
  const char ACCEPTED_METHOD[] = "GET";
  // YOUR CODE HERE

  /* Read the request from the socket.
     In case of error, syslog it, close the socket, and return false */
  // YOUR CODE HERE

  /* Extract the method */
  // YOUR CODE HERE
  
  /* Is it the GET method? */
  // YOUR CODE HERE

  /* Extract the document */
  // YOUR CODE HERE

  /* Process the GET method */
   // YOUR CODE HERE

  // JUST FOR TESTING; PLEASE REMOVE
  const char *header = "HTTP/1.1 300 Multiple Choices\r\n\r\nI am a Ybruf, yo!";
  write(sock_id, header, strlen(header));
  
  close(sock_id);  
  return true;  
}
