#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include "server.h"

#define BUF_SIZE 2048
struct stat file_stat;                                          /* argument to fstat */

int main(int argc, char **argv){
  int numOfVideos = 3;                                          /* number of available videos at the server */
  char *video_name1 = "BigBuckBunny.mp4";                       /* video 1 name */
  char *video_name2 = "Car.mp4";                                /* video 2 name, dummy. segments not available */
  char *video_name3 = "Agrasagar.mp4";                          /* video 3 name, dummy. segments not available */
  char *mpd_name1 = "BigBuckBunny_4s.mpd";                      /* MPD for video 1 */
  char *initSeg1 = "bunny_45226bps/BigBuckBunny_4s_init.mp4";   /* init segment for video 1 */
  int numOfSegments1 = 150;                                     /* total video segments of video 1 */
  int sockfd, connfd;                                           /* socket discrptors */
  int filefd;                                                   /* file descriptor */
  char *video;                                                  /* requested video name */
  char *mpd;                                                    /* MPD of video selected by client */
  char *initSeg;                                                /* init segment of video selected by client */
  char *videoSeg;                                               /* currenty video segement asked by client */
  int numOfSegments;                                            /* number of video segments of selected video */
  int optionNumber;                                             /* option recvd from client */
  char *badOpt = "Bad option.";                                 /* msg to client if wrong video option is selected by client */
  int kk =0;                                                    /* loop variable */
  int pid;                                                      /* child pid */
  char availableVideos[200];                                    /* all available videos list to be sent to client */

  if (argc != 3){
    fprintf(stderr, "Usage: ./server <IPaddress> <port>\n");
    exit(EXIT_FAILURE);
  }
  /* create and bind socket */
  sockfd = bindSocket(argv[1], argv[2]);
  /* wait for a client to connect */
  if (listen(sockfd, 10) < 0) {
    fprintf(stderr, "listen failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  signal(SIGCHLD, sig_chld);
  while (1){
    if ( (connfd = accept(sockfd, NULL, NULL)) < 0) {
      fprintf(stderr, "accept failed: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    /* each client handling in seperate process */
    pid = fork();
    if (pid < 0){              //Fork Error
      close(connfd);
      perror("fork");
      exit(EXIT_FAILURE);
    }
    if (pid == 0){            //Child process created successfully
      close(sockfd);          //Child will not listen
      snprintf(availableVideos, sizeof(availableVideos), "%s %s\n%s %s\n%s %s\n", "[1]", video_name1, "[2]", video_name2, "[3]", video_name3);
      do {
        /* send available video list to client */
        send_name_length(connfd, availableVideos);
        sendString(connfd, availableVideos);
        /* recv video number selected by client */
        optionNumber = recv_opt_num(connfd);
        if (optionNumber == 1){
          send_name_length(connfd, video_name1);
          sendString(connfd, video_name1);
          video = video_name1;
        } else if (optionNumber == 2){
          send_name_length(connfd, video_name2);
          sendString(connfd, video_name2);
          video = video_name1;
        } else if (optionNumber == 3){
          send_name_length(connfd, video_name3);
          sendString(connfd, video_name3);
          video = video_name1;
        } else {
          send_name_length(connfd, badOpt);
          sendString(connfd, badOpt);
        }
      } while ( (optionNumber > numOfVideos) || (optionNumber <=0) );
      /* select mpd and init for requested video. */
      if (strcmp(video, video_name1)==0){
        mpd = mpd_name1;
        initSeg = initSeg1;
        numOfSegments = numOfSegments1;
      } else if (strcmp(video, video_name1)==0){ /*dummy */
        mpd = mpd_name1;
        initSeg = initSeg1;
        numOfSegments = numOfSegments1;
      } else if (strcmp(video, video_name1)==0){ /* dummy */
        mpd = mpd_name1;
        initSeg = initSeg1;
        numOfSegments = numOfSegments1;
      }
      /* sned MPD */
      send_name_length(connfd, mpd);
      sendString(connfd, mpd);
      filefd = get_filedes(mpd);
      get_file_stat(filefd);
      send_num(connfd, file_stat.st_size);
      send_file(connfd, filefd, file_stat.st_size);

      /* send number of segments of video */
      send_num(connfd, numOfSegments);

      /* Send init segement. Init segement is same for all video qualities */
      filefd = get_filedes(initSeg);
      get_file_stat(filefd);
      send_num(connfd, file_stat.st_size);
      send_file(connfd, filefd, file_stat.st_size);

      /* continue untill all segments are sent to client */
      while(kk < numOfSegments){
        kk++;
        /* receive segment name */
        videoSeg = recv_req(connfd);
        /* get file stats */
        filefd = get_filedes(videoSeg);
        get_file_stat(filefd);
        /* send segment size */
        send_num(connfd, file_stat.st_size);
        /* send segment */
        send_file(connfd, filefd, file_stat.st_size);
      }
      kk = 0;
      //printf("Send video %s complete.\n", video);
      close(connfd);
      return 0;
    }
  }
  close(sockfd);
  //printf("Bye from server\n");
  return 0;
}

int bindSocket(char *addrs, char *port){
  struct addrinfo hints, *servAddr;
  int status;
  int sockDes;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if ((status = getaddrinfo(addrs, port, &hints, &servAddr)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(EXIT_FAILURE);
  }
  sockDes = socket(servAddr->ai_family, servAddr->ai_socktype,servAddr->ai_protocol);
  if (bind(sockDes, servAddr->ai_addr, servAddr->ai_addrlen) < 0) {
    fprintf(stderr, "unable to bind to socket: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(servAddr);
  return sockDes;
}

char *recv_req(int socket_des){
  static char buffer[BUF_SIZE];
  memset(buffer, 0, sizeof(buffer));
  if (read(socket_des, buffer, sizeof(buffer)) < 0) {
    fprintf(stderr, "Read request failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  return buffer;
}

void send_name_length(int socket_des, char *file_name){
  int name_length = 0 ;
  name_length = htonl(strlen(file_name));
  if (write(socket_des, &name_length, sizeof(int)) < 0){
    fprintf(stderr, "Error on writing name length --> %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void sendString(int socket_des, char *nameStr){
  if (write(socket_des, nameStr, strlen(nameStr)) < 0){
    fprintf(stderr, "Error on writing string --> %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

int recv_opt_num(int socket_des){
  int optNum=0;
  if ( read(socket_des, &optNum, sizeof(int)) < 0 ){
    fprintf(stderr, "Error on reading option number --> %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  optNum = ntohl(optNum);
  return optNum;
}

int get_filedes(char *file_name){
  int file_des = open(file_name, O_RDONLY);
  if (file_des < 0){
    fprintf(stderr, "Error opening file --> %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  return file_des;
}

void get_file_stat(int file_des){
  if (fstat(file_des, &file_stat) < 0){
    fprintf(stderr, "Error fstat --> %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void send_num(int socket_des, int num){
  num = htonl(num);
  if (write(socket_des, &num, sizeof(int)) < 0){
    fprintf(stderr, "Error writing number --> %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void send_file(int socket_des, int file_des, int size){
  int read_return = 0;
  char buffer[BUF_SIZE];
  while (1) {
    memset(buffer,0,sizeof(buffer));
    read_return = read(file_des, buffer, sizeof(buffer));
    //printf("read return %d\n", read_return);
    if (read_return == 0) break;
    if (read_return == -1){
      fprintf(stderr, "Error reading file --> %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (write(socket_des, buffer, read_return) < -1) {
      fprintf(stderr, "Error writing file to socket --> %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
}

// Process SIGCHLD: when child dies, must use waitpid to clean it up
// otherwise it will remain as zombie
void sig_chld(int signo){
  pid_t pid;
  int stat;
	(void) signo;              // silence warning
  while( (pid = waitpid(-1, &stat, WNOHANG)) > 0 ){
    //printf("child %d terminated\n", pid);
  }
  return;
}
