#include <fcntl.h>

#include "ybruf.h"

// Memory configuration parameters
static const int MAX_RQ_SIZE = 1048576; // 1MB
static const int MAX_DATA_SIZE = 1048576; // 1MB
const char* error_bad_request = "HTTP/1.1 400 Bad Request\r\n\r\n";
const char* error_not_allowed = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
const char* error_fnot_found = "HTTP/1.1 404 File Not Found\r\n\r\n";
const char* success = "HTTP/1.1 200 OK\r\n\r\n";


bool write_http_header(int sock_id, const char *status, const char *msg)
{ 
    int write_status_code = write(sock_id,status,strlen(status));

    if(write_status_code < 0){ //error in writing, report it
      syslog(LOG_ERR, "write status: %s",strerror(errno));
      return false;
    }
    
    if (msg!=NULL){ //message can also be NULL
      int write_message_code = write(sock_id,msg,strlen(msg));

      if (write_message_code < 0){ //if above write failed report it
        syslog(LOG_ERR, "write message code: %s",strerror(errno));
        return false;
      }
    }

  return true;
}

bool process_GET(int sock_id, char *doc)
{
  char data[MAX_DATA_SIZE];
  
  if (*doc=='/'){//if first is /, move pointer one position to the right (i.e. remove /)
    doc++;
  }

  if (strlen(doc)==0){//if no doc, put default index.html
    doc = DEFAULT_DOC;
  }

  if (-1 == chdir(APP_WD)) {
    syslog(LOG_ERR,"chdir(): %s",strerror(errno)); //go to /tmp where images/files are
    return false;
  }

  FILE *display_file;
  
  if (!(display_file = fopen(doc, "r"))) {// error in opening the file, report it
    write_http_header(sock_id,error_fnot_found,error_fnot_found);
    return false;
  }
  
  int write_success = write_http_header(sock_id,success,NULL);//write success header in socket

  if (write_success==false){
    return false;
  }

  
//------------------fread--------------------------------------------------
  // fseek(display_file,0L,SEEK_END); //move file pointer to the end
  // long int size = ftell(display_file);//find the position of the pointer
  // fseek(display_file,0,SEEK_SET); //move it back to the start of file
  
  // char file_buffer[size];
  // int read_file = fread(&file_buffer,sizeof(char),size,display_file); //doesn't set errno? How detect errors
  // write(sock_id,file_buffer,read_file); //write file to socket. read_file is amount read


//---------------------(proper) read----------------------------------------------


  int fid = fileno(display_file); //getting the file descriptor (read needs file handler)
  int read_status;

  do{
    read_status = read(fid,data,MAX_RQ_SIZE);
    int write_status = write(sock_id,data,read_status);

    if (read_status < 0){ //reading file fails
      syslog(LOG_ERR,"Read file: %s", strerror(errno));
      return false;
    }

    if (write_status < 0){
      syslog(LOG_ERR,"Write file in socket: %s", strerror(errno));
      return false;
    }

    if (write_status!=read_status){ //if we wrote less than what was read then error occured
      syslog(LOG_ERR, "write(): %s",strerror(errno));
      return false;
    }

  }while(read_status > 0); //0 is EOF


  fclose(display_file);
  return true;
}

bool process_request(int sock_id)
{
  
    const char ACCEPTED_METHOD[] = "GET";
    char buffer[MAX_RQ_SIZE]; 

    int socket_read = read(sock_id,buffer,MAX_RQ_SIZE); //read request
    
    if (socket_read < 0){
      syslog(LOG_ERR,"read():%s",strerror(errno));
      close(sock_id);
      return false;
    }

    char* method = strtok(buffer," ");//extract method

    if (method==NULL){ //method cannot be extracted
      write_http_header(sock_id,error_bad_request,NULL);
      close(sock_id);
      return false;
    }

    int method_cmp = strcasecmp(method,ACCEPTED_METHOD);//compare with ACCEPTED_METHOD

    if(method_cmp!=0){ //method is not GET
      write_http_header(sock_id,error_not_allowed,NULL);
      close(sock_id);
      return false;
    }
    
    char* file_name = strtok(NULL," ");

    if (file_name==NULL){ //extracting file name fails
      write_http_header(sock_id,error_bad_request,NULL);
      close(sock_id);
      return false;
    }

  bool process_get = process_GET(sock_id, file_name);
  close(sock_id);

  if (process_get==0){//0 means false
    return false;  
  }

  return true;
}
