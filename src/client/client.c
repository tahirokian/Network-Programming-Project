#include <sys/socket.h>  // defines socket, connect, ...
#include <netinet/in.h>  // defines sockaddr_in
#include <string.h>      // defines memset
#include <stdio.h>       // defines printf, perror, ...
#include <arpa/inet.h>   // inet_pton, ...
#include <unistd.h>      // read, ...
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "client.h"
#include "demuxing.c"
#include "mpd_parser.c"

#define BUF_SIZE 2048

int main(int argc, char **argv){
  int sockfd_tcp;                         /* tcp socket descriptor */
  char *file_name;                        /* received mpd file name */
  int file_size=0;                        /* received file size */
  int name_len=0;                         /* length of the name of file to be received */
  Mpd *mpdData;                           /* mpd parsed data holder */
  int numMpdEntr=0;                       /* number of entries of mpd parsed data holder */
  int numOfSegments=0;                    /* number of segments of video */
  int optionNum;                          /* video number selected by user */
  char *vidNames;                         /* Videos available on server */
  char *selectedVid;                      /* video selected by user */
  int compareRes;                         /* comparison result if selected option is incorrect */

  if (argc != 3){
    fprintf(stderr, "Usage: ./client <IPaddress> <port>\n");
    exit(EXIT_FAILURE);
  }
  /* connect to the server on ip and port enetred by user */
  sockfd_tcp= connectServer(argv[1], argv[2]);

  /* tell server which video you wana see by selectin an integer option. Error check only for integers */
  do {
    name_len = recvNum(sockfd_tcp);
    vidNames = recvString(sockfd_tcp, name_len);
    printf("Availabale videos are\n%s", vidNames);
    printf("Please enter video number: ");
    scanf("%d", &optionNum);
    sendNum(sockfd_tcp, optionNum);
    name_len = recvNum(sockfd_tcp);
    selectedVid = recvString(sockfd_tcp, name_len);
    compareRes = strcmp("Bad option.", selectedVid);
    if (strcmp("Bad option.", selectedVid) == 0){
      printf("Bad option selected. Try again...\n");
    }
  } while (compareRes == 0);

  printf("Your Selected video is: %s\n", selectedVid);
  /*Receive mpd */
  name_len = recvNum(sockfd_tcp);
  file_name = recvString(sockfd_tcp, name_len);
  //printf("Received mpd file: %s\n", file_name);
  file_size = recvNum(sockfd_tcp);
  //printf("MPD file size: %d\n", file_size);
  recv_file(sockfd_tcp, file_size, file_name);

  /*Parse MPD. Parsing logic is implemented in mpd_parser.c*/
  mpdData = mpdParser(file_name, &numMpdEntr);
  //printf("Number of MPD entries are %d\n", numMpdEntr);

  /* receive number of segments of the selected video */
  numOfSegments = recvNum(sockfd_tcp);
  //printf("Number of segments: %d\n", numOfSegments);

  /*Video reception and choosing video quality logic is implemented in demuxing.c */
  mpeg_decode_TS(mpdData, sockfd_tcp, numOfSegments);
  printf("Video playing finished\n");

  close(sockfd_tcp);
  return 0;
} //main ends

int connectServer(char *servAddr, char *port){
  struct addrinfo hints, *res, *p;
  int status;
  int socketDes;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if ((status = getaddrinfo(servAddr, port, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(EXIT_FAILURE);
  }
  p = res; //So that memory can be released afterwards
  do{
    socketDes = socket(res->ai_family, res->ai_socktype,res->ai_protocol);
    if (socketDes < 0)
      continue;       /* ignore this one */
    if (connect(socketDes, res->ai_addr, res->ai_addrlen) == 0)
      break;
    close(socketDes);  /* ignore this one */
  } while( (res = res->ai_next) != NULL );
  if (res == NULL) {      /* errno set from final connect() */
    fprintf(stderr, "tcp connect error for %s, %s\n", servAddr, port);
    socketDes = -1;
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(p);
  return socketDes;
}


void sendString(int socket_fd, char *str){
  if ( write(socket_fd, str, strlen(str)) < 0 ){
    perror("write error");
    exit(EXIT_FAILURE);
  }
  return;
}

int recvNum(int socket_fd){
  int num=0;
  if ( read(socket_fd, &num, sizeof(int)) < 0 ){
    perror("read error");
    exit(EXIT_FAILURE);
  }
  num = ntohl(num);
  return num;
}

char *recvString(int socket_fd, int sz){
  static char str[100];
	memset(str,0,sizeof(str));
  if ( read(socket_fd, str, sz) < 0 ){
    perror("read error");
    exit(EXIT_FAILURE);
  }
  return str;
}

void recv_file(int socket_fd, int file_size, char *file_name){
  int remain_data=0, bytes_recvd=0, total_bytes_recvd=0, buf_siz=2048;
  FILE *fp = fopen(file_name, "w+");
  char buffer[BUF_SIZE];
  if (fp == NULL){
    perror("file open error");
    exit(EXIT_FAILURE);
  }
  remain_data = file_size;
  memset(buffer,0,sizeof(buffer));
  while ( ((bytes_recvd = read(socket_fd, buffer, buf_siz)) > 0) && (remain_data > 0) ){
    fwrite(buffer, sizeof(char), bytes_recvd, fp);
    remain_data -= bytes_recvd;
    total_bytes_recvd += bytes_recvd;
    if (remain_data < buf_siz){
      buf_siz = remain_data;
    }
    memset(buffer,0,sizeof(buffer));
  }
  //printf("Total bytes recvd: %d\n", total_bytes_recvd);
  fclose(fp);
  return;
}

void sendNum(int socket_des, int optNum){
  optNum = htonl(optNum);
  if (write(socket_des, &optNum, sizeof(int)) < 0){
    perror("sending number error");
    exit(EXIT_FAILURE);
  }
}
