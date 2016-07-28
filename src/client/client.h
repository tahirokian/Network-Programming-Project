#ifndef NWP_PROJECT_H
#define NWP_PROJECT_H

#include <stdint.h>

typedef struct mpd_data {
    char duration[30];
    char media[100];
    char initialization[100];
    char id[30];
    char mimeType[30];
    char codecs[30];
    char width[30];
    char height[30];
    char frameRate[30];
    char sar[30];
    char startWithSAP[30];
    char bandwidth[30];
} Mpd;

/* create a tcp socket, connect to the server, returns socket descriptor */
int connectServer(char *, char *);
/* send an integer value to the server */
void sendNum(int , int );
/* send a string to the server */
void sendString(int , char *);
/* receive an integer value from the server */
int recvNum(int );
/* receive a string from the server */
char *recvString(int , int);
/* receive file, video segment from the server */
void recv_file(int , int , char *);
/* parse MPD */
Mpd * mpdParser(char *, int *);
/* video segment name generator */
char *segNameGen(char *media,char *band, int segnum);
/* video segment request, dummy video player logic */
int mpeg_decode_TS(Mpd *, int, int );
/* dummy video player based on timer */
void *ptimer();

#endif
