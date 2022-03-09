#include <fcntl.h>

#include "ybruf.h"

// Memory configuration parameters
static const int MAX_RQ_SIZE = 1048576; // 1MB
static const int MAX_DATA_SIZE = 1048576; // 1MB

bool write_http_header(int sock_id, const char *status, const char *msg)
{

  if (msg!=NULL){
    char message[strlen(status)+strlen(msg)];
    strcpy(message,status);
    strcat(message,msg);
    write(sock_id,message,strlen(message));
  }
  // else{
  //   write(sock_id);
  // }
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
  char buffer[MAX_RQ_SIZE];
  const char* error_header = "HTTP/1.1 400 Bad Request\r\n\r\n";

    int socket_read = read(sock_id,buffer,MAX_RQ_SIZE);
    if (socket_read < 0){
      syslog(LOG_ERR,"read():%s",strerror(errno));
    }


    char* method = strtok(buffer," ");
    if (strcmp(method,"")){
      write_http_header(sock_id,error_header,NULL);
      return false;
    }

    char* file_name = strtok(NULL," ");
    
    // if (strcmp(++file_name),""){

    // }

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
