#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>

#include "ybruf.h"

// Memory configuration parameters
static const int MAX_RQ_SIZE = 1048576; // 1MB
static const int MAX_DATA_SIZE = 1048576; // 1MB

// Request processor
static bool write_http_header(int sock_id,
			      const char *status,
			      const char *msg)
{
  static const char *RNRN = "\r\n\r\n";
  if (   strlen(status)       != write(sock_id, status, strlen(status))
      || strlen(RNRN)         != write(sock_id, RNRN, strlen(RNRN))
      || (msg && (strlen(msg) != write(sock_id, msg, strlen(msg))))) {
    syslog(LOG_ERR, "write(): %s", strerror(errno));
    return false;
  }
  return true;
}

static bool process_dir(int sock_id, char *doc)
{
  // Open the directory for reading
  // YOUR CODE GOES HERE

  // Write the header - we seem to be ok
  // Technically, this is not correct. We must first fully read the directory 
  // YOUR CODE GOES HERE

  // Define the template
  // YOUR CODE GOES HERE

  errno = 0; // Assume no errors
  // YOUR CODE GOES HERE
    
  if (errno) {
    // Too late to complain!
  }
  
  return errno == 0;
}

static bool process_GET(int sock_id, char *doc)
{
  // Check if the file exists, and get its information
  struct stat statbuf;
  if (-1 == stat(doc, &statbuf)) {
    write_http_header(sock_id, PROTO "404 File Not Found", 
		      "File Not Found");
    return false;
  }

  // Process directory listing
  if (statbuf.st_mode & S_IFDIR)
    return process_dir(sock_id, doc);

  // Process file 
  int infile = open(doc, O_RDONLY);
  if (-1 == infile) {
    write_http_header(sock_id, PROTO "403 Forbidden", "Forbidden");
    return false;
  } else {
    if (!write_http_header(sock_id, PROTO "200 OK", NULL))
      return false;

    // Copy the file to the socket
    int size;
    char data[MAX_DATA_SIZE];

    while (0 < (size = read(infile, data, sizeof(data)))
	   && (size == write(sock_id, data, size)));
    close(infile);  
  }
  return true;
}

bool process_request(int sock_id)
{
  const char ACCEPTED_METHOD[] = "GET";
  char data[MAX_RQ_SIZE];

  /* Read the request from the socket.
     In case of error, syslog it, close the socket, and return false */
  int size = read(sock_id, data, sizeof(data));
  if (size <= 0) {
    syslog(LOG_ERR, "read(): %s", strerror(errno));
    close(sock_id);
    return false;
  }
  data[size] = 0;

  /* Extract the method */
  char *request = strtok(data, " ");
  if (!request) {
    write_http_header(sock_id, PROTO "400 Bad Request", "Bad request");
    close(sock_id);  
    return false;
  }
  
  /* Is it the GET method? */
  if (strcasecmp(request, ACCEPTED_METHOD)) {
    write_http_header(sock_id, PROTO "405 Method Not Allowed",
		      "Method Not Allowed");
    close(sock_id);  
    return false;
  }

  /* Extract the document */
  char *doc = strtok(NULL, " ");
  if (!doc) {
    write_http_header(sock_id, PROTO "400 Bad Request", "Bad Request");
    close(sock_id);  
    return false;
  }

  /* Process the GET method */
  doc[-1] = '.';
  process_GET(sock_id, doc - 1);
  close(sock_id);
  
  return true;  
}
