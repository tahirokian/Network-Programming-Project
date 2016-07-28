#ifndef NWP_PROJECT_H
#define NWP_PROJECT_H

/* creat a socket, bind it and return socket descriptor */
int bindSocket(char *, char *);
/* receive name segment asked by client */
char *recv_req(int );
/* send length of the video or file name to be sent */
void send_name_length(int , char *);
/* sends string  */
void sendString(int , char *);
/* recv option number selected by client */
int recv_opt_num(int );
/* open a file and get a desccriptor for the file */
int get_filedes(char *);
/* gives file size */
void get_file_stat(int );
/* send an inetger value (size) to the client */
void send_num(int , int );
/* send video segments, file to the client */
void send_file(int, int, int);
/* child process handler */
void sig_chld(int);

#endif
