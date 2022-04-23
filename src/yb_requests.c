#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>

#include "ybruf.h"

// Memory configuration parameters
// static const int MAX_RQ_SIZE = 1048576;   // 1MB
static const int MAX_DATA_SIZE = 1048576; // 1MB

// Request processor
static bool write_http_header(int sock_id,
                              const char *status,
                              const char *msg)
{
  static const char *RNRN = "\r\n\r\n";
  if (strlen(status) != write(sock_id, status, strlen(status)) || strlen(RNRN) != write(sock_id, RNRN, strlen(RNRN)) || (msg && (strlen(msg) != write(sock_id, msg, strlen(msg)))))
  {
    syslog(LOG_ERR, "write(): %s", strerror(errno));
    return false;
  }
  return true;
}

static char *process_dir(char *doc, int *len)
{
  // Open the directory for reading
  DIR *cwd = opendir(doc);
  if (!cwd)
    return NULL;

  struct dirent *entry;

  // Define the template
  int dirSize = 0;
  static const char *PRE_FMT = "<html><h1>Directory Index</h1>\n<ul>";
  static const char *LINE_FMT = "<li><a href=\"%s%s\">%s</a>\n";
  static const char *POST_FMT = "</ul></html>";

  char row[strlen(LINE_FMT) + 2 * NAME_MAX + 1];
  dirSize += strlen(PRE_FMT) + strlen(POST_FMT); 

  errno = 0;                                     // Assume no errors
  // get the size of the string
  while ((entry = readdir(cwd)))
  {
    if (strcmp(entry->d_name, "."))
    { // No self references
      sprintf(row, LINE_FMT,
              entry->d_name,
              entry->d_type == DT_DIR ? "/" : "",
              entry->d_name);
      dirSize += strlen(row);
    }
  }
  
  if (errno!=0){
    syslog(LOG_ERR, "readdir(): %s", strerror(errno));
    closedir(cwd);
    return NULL;
  }
  *len = dirSize;

  char dirbuff[dirSize];

  // construct the string with static buffer
  strcpy(dirbuff, PRE_FMT); // need to copy not strcat because there's nothing in there yet
  rewinddir(cwd);           // start again from begining

  while ((entry = readdir(cwd)))
  {
    if (strcmp(entry->d_name, "."))
    {
      sprintf(row, LINE_FMT,
              entry->d_name,
              entry->d_type == DT_DIR ? "/" : "",
              entry->d_name);
      strcat(dirbuff, row);
    }
  }

  if (errno!=0){
    syslog(LOG_ERR, "readdir(): %s", strerror(errno));
    closedir(cwd);
    return NULL;
  }
  
  strcat(dirbuff, POST_FMT);

  // dup returns pointer to dirbuff
  char *data = strdup(dirbuff);

  if (data==NULL){
    syslog(LOG_ERR, "strdup(): %s", strerror(errno));
    closedir(cwd);
    return NULL;
  }

  closedir(cwd);
  return data;
}

static char *process_file(char *doc, int *len)
{
  int infile = open(doc, O_RDONLY);
  if (-1 == infile)
    return NULL;
  // find file info
  struct stat statbuf;
  if (-1 == stat(doc, &statbuf)){
    syslog(LOG_ERR, "stat: %s", strerror(errno));
    return NULL;
  }
  *len = statbuf.st_size;

  // allocate appropriate space for file content
  char *data = malloc(statbuf.st_size);

  int size;
  if (data!=NULL){
    while (0 < (size = read(infile, data, *len))); //read it
    if (size==-1){
      syslog(LOG_ERR, "read(): %s", strerror(errno));
      close(infile);
      return NULL;
    }
    close(infile); 
  }
  else{
    syslog(LOG_ERR, "malloc(): %s", strerror(errno));
    close(infile);
    return NULL;
  }

  return data;
}

static bool process_GET(int sock_id, char *doc)
{
  int len;
  char *data = cache_lookup(doc, &len);
  bool found = false;

  if (!data)
  { // We haven't seen this doc before
    // Check if the doc exists, and get its information
    struct stat statbuf;
    if (-1 == stat(doc, &statbuf))
    {
      write_http_header(sock_id, PROTO "404 File Not Found",
                        "File Not Found");
      return false;
    }

    if (statbuf.st_mode & S_IFDIR)
      data = process_dir(doc, &len); // Process directory listing
    else if (statbuf.st_mode & S_IFREG)
      data = process_file(doc, &len); // Process regular file
    // else ignore

    if (!data)
    {
      write_http_header(sock_id, PROTO "404 File Not Found",
                        "File Not Found");
      return false;
    }

    // Cache the content
    // If this line fails, it's ok!
    cache_insert(doc, data, len);
  }
  else
    found = true;

  // puts("hi");
  // printf("doc: %s\ndata: %s\n", doc, data);
  // Write the header
  if (!write_http_header(sock_id, PROTO "200 OK", NULL))
  {
    if (found) /* Avoid memory leaks */
      free(data);
    return false;
  }

  // Write the data
  if (len != write(sock_id, data, len))
  {
    syslog(LOG_ERR, "write(): %s", strerror(errno));
    if (found) /* avoid memory leaks */
      free(data);
    return false;
  }

  if (found) /* Avoid memory leaks */
    free(data);
  return true;
}

bool process_request(int sock_id)
{
  const char ACCEPTED_METHOD[] = "GET";
  char data[MAX_RQ_SIZE];

  /* Read the request from the socket.
     In case of error, syslog it, close the socket, and return false */
  int size = read(sock_id, data, sizeof(data));
  if (size <= 0)
  {
    syslog(LOG_ERR, "read(): %s", strerror(errno));
    close(sock_id);
    return false;
  }
  data[size] = 0;

  /* Extract the method */
  char *request = strtok(data, " ");
  if (!request)
  {
    write_http_header(sock_id, PROTO "400 Bad Request", "Bad request");
    close(sock_id);
    return false;
  }

  /* Is it the GET method? */
  if (strcasecmp(request, ACCEPTED_METHOD))
  {
    write_http_header(sock_id, PROTO "405 Method Not Allowed",
                      "Method Not Allowed");
    close(sock_id);
    return false;
  }

  /* Extract the document */
  char *doc = strtok(NULL, " ");
  if (!doc)
  {
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