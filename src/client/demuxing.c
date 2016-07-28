/*
 * Copyright (c) 2012 Stefano Sabatini
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * Demuxing and decoding example.
 *
 * Show how to use the libavformat and libavcodec API to demux and
 * decode audio and video data.
 * @example demuxing_decoding.c
 */

 #include <libavutil/imgutils.h>
 #include <libavutil/samplefmt.h>
 #include <libavutil/timestamp.h>
 #include <libavformat/avformat.h>
 #include <stdbool.h>
 #include <pthread.h>
 #include <sys/time.h>
 #include <stdlib.h>
 #include "client.h"
 #include "segmentname_generator.c"
 #include <signal.h>

 #define DIV_ROUND_CLOSEST(x, divisor)(                  \
 {                                                       \
         typeof(divisor) __divisor = divisor;            \
         (((x) + ((__divisor) / 2)) / (__divisor));      \
 }                                                       \
 )

 #define SEC2PICO UINT64_C(1000000000000)
 #define SEC2MILI 1000
 #define MAX_WRITE_SIZE 2048

static AVFrame *frame = NULL;
static AVPacket pkt;

Mpd * mpdData;                          /* mpd parsed datd container */
int segSize=0;                          /* segment size value */
int remainData=0;                       /* remaining segment data size */
int numSegments=0;                      /* total number of segments for the current video */
int yy=0;                               /* currently requested video segment serial number */
int zz = 0;                             /* checks if number of requested segments is less than total number of segemnts of the video */
bool x=false;                           /* for creating thread at the start */
unsigned long long playtime_downloaded;
unsigned long long playtime_played;
signed long long playtime_diff;
unsigned long long resume=0;
unsigned long long vidlen_total=-1;
pthread_t tids;                           /* thread ID, single thread for ptimer */
char tempFile[100]="temp.mp4";            /* saved  video downloaded */
char Res_Report[100]="Res_Report.dat";    /* saves results for plotting */
FILE *fp;                                 /* video temp file pointer */
FILE *res_report;                         /* results file pointer */
int res_240p=0;                           /* counter for 240p */
int res_360p=0;                           /* counter for 360p */
int res_720p=0;                           /* counter for 720p */
int res_1080p=0;                          /* counter for 1080p */

static int read_packet(void *opaque, uint8_t *buf, int buf_size) {
  size_t ret;
  int * ptr_sockdes = ((int *) opaque);
  char *str1;                             /* contains requested segment name */
  char str2[50];                          /* for storing quality results data to the file */
  char *media=mpdData[0].media;           /* segment name format container */
  /* if remain data of segment is less than buffer size, receive only remain amount of data. */
  if (remainData <= buf_size){
    buf_size = remainData;
  }
  if ( (ret = read(*ptr_sockdes, buf, buf_size)) < 0){
    exit(EXIT_FAILURE);
  }
  remainData -= ret;
  /* write only when there is something available on read buffer */
  if(ret!=0){
    fwrite(buf , sizeof(char) , ret , fp);
  }
  /* create thread only once */
  if(x==false){
    //launch thread for timer
    if (pthread_create(&tids, NULL, &ptimer, (void *) (intptr_t) buf_size) != 0) {
      perror("pthread_create");
      exit(EXIT_FAILURE);
    }
    x=true;
  }
  /* if segment is completed received, ask for new segment based on network condition */
  if (remainData == 0) {
    playtime_diff=playtime_downloaded-playtime_played;
    /* pause download if downloading is fast, then restart */
    /*if(playtime_diff> 30000 ){
      //printf("||||||||||||||||||||||Download paused||||||||||||||||||||\n");
      while(playtime_diff>3500){
        playtime_diff=playtime_downloaded-playtime_played;
        //printf("playtime_diff: %llu\n",playtime_diff);
        //printf("~~~~~~~~~~~~player is playing at %llu milliseconds \n", playtime_played);
      }
    }*/
    /* the following logic requests video segment based on time difference between current play time and download video time */
    yy++;
    str1=NULL;
    if (playtime_diff <5000){ /* if play time and downloaded segment's last TS is less than 5sec */
      res_240p++;
      snprintf(str2, sizeof(str2), "%llu %s\n", playtime_downloaded,"240p");
      str1=segNameGen(media,mpdData[1].bandwidth,yy);           /* mpdData[1] --> bandwidth="45226" */
    } else if (playtime_diff >=5000 && playtime_diff<13000){    /* if play time and downloaded segment's last TS is between 5sec and 13sec */
      res_360p++;
      snprintf(str2, sizeof(str2), "%llu %s\n", playtime_downloaded,"360p");
      str1=segNameGen(media,mpdData[4].bandwidth,yy);           /* mpdData[4] --> bandwidth="177437" */
    } else if (playtime_diff >=13000 && playtime_diff <19000){  /* if play time and downloaded segment's last TS is between 13sec and 19sec */
      res_720p++;
      snprintf(str2, sizeof(str2), "%llu %s\n", playtime_downloaded,"720p");
      str1=segNameGen(media,mpdData[11].bandwidth,yy);          /* mpdData[11] --> bandwidth="782553" */
    } else if (playtime_diff >=19000){                          /* if play time and downloaded segment's last TS is more than 19sec */
      res_1080p++;
      snprintf(str2, sizeof(str2), "%llu %s\n", playtime_downloaded,"1080p");
      str1=segNameGen(media,mpdData[15].bandwidth,yy);          /* mpdData[15] --> bandwidth="2087347" */
    }
    fwrite(str2 , sizeof(char) , strlen(str2) , res_report);
    /*Send segment request*/
    if (zz < numSegments){
      printf("Video requested: %s\n", str1);
      sendString(*ptr_sockdes, str1);
      zz++;
      segSize = recvNum(*ptr_sockdes);
      remainData = segSize;
    }
  }
  return ret;
}//read_packet ends

int mpeg_decode_TS (Mpd *mpdData1, int socket_des, int numOfSegments){
  int ret = 0;
  mpdData = mpdData1;                     /* assign value to global variable */
  numSegments = numOfSegments;            /* assign value to global variable */
  segSize = recvNum(socket_des);          /* get segment size of init file for server */
  remainData = segSize;                   /* initially remain data is same as size of segment to be received */
  fp = fopen(tempFile, "w");              /* video temp file pointer*/
  res_report = fopen(Res_Report, "w");    /* reults file pointer */

  /* register all formats and codecs */
  av_register_all();

  /*allocate a buffer for the data read from file/socket*/
  void *buff = av_malloc(MAX_WRITE_SIZE);

  /*allocate and initialize an AVIOContext for buffered I/O. read_packet is the
    function where the buffer is refilled, the file pointer is passed here so
    it can be read inside the function*/
  AVIOContext *avio = avio_alloc_context(buff, MAX_WRITE_SIZE, 0, &socket_des, read_packet, NULL, NULL);
  if (avio == NULL) {
 	  exit(EXIT_FAILURE);
  }

	AVFormatContext *fmt_ctx = avformat_alloc_context();
	if (fmt_ctx == NULL) {
		exit(EXIT_FAILURE);
	}
	fmt_ctx->pb = avio;

  /* open input file, and allocate format context */
  if (avformat_open_input(&fmt_ctx, NULL, NULL, NULL) < 0) {
    //fprintf(stderr, "Could not open source file %s\n", file_name);
    fprintf(stderr, "Could not open source stream\n");
    exit(EXIT_FAILURE);
  }

  /* retrieve stream information */
  if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
    fprintf(stderr, "Could not find stream information\n");
    exit(EXIT_FAILURE);
  }
  int videoStreamIdx = -1;
  int audioStreamIdx = -1;
  unsigned int i;
  for (i = 0; i < fmt_ctx->nb_streams; i++) {
	   if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
       videoStreamIdx = i;
     } else if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
       audioStreamIdx = i;
	   }
  }
  unsigned long long vtb __attribute__ ((unused)) = 0;
  if (videoStreamIdx != -1) {
    int vnum = fmt_ctx->streams[videoStreamIdx]->time_base.num;
	  if (vnum > (int) (UINT64_MAX / SEC2PICO)) {
      exit(EXIT_FAILURE);
	  }
	  int vden = fmt_ctx->streams[videoStreamIdx]->time_base.den;
	  vtb = DIV_ROUND_CLOSEST(vnum * SEC2PICO, vden);
  }
	unsigned long long atb = 0;
	if (audioStreamIdx != -1) {
    int anum = fmt_ctx->streams[audioStreamIdx]->time_base.num;
		if (anum > (int) (UINT64_MAX / SEC2PICO)) {
			exit(EXIT_FAILURE);
		}
		int aden = fmt_ctx->streams[audioStreamIdx]->time_base.den;
		atb = DIV_ROUND_CLOSEST(anum * SEC2PICO, aden);
	}

  /* Extracting the timestamp of the received frame and printing it */
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	while (( ret = av_read_frame(fmt_ctx, &pkt) ) >= 0) {
		if (pkt.stream_index == videoStreamIdx) {
			if (pkt.dts > 0) {
        playtime_downloaded=(pkt.dts * vtb) / (SEC2PICO / SEC2MILI);
        //printf("Video TS = %llu\n",(pkt.dts * vtb) / (SEC2PICO / SEC2MILI));
			}
		} else if (pkt.stream_index == audioStreamIdx) {
			if (pkt.dts > 0) {
				printf("Audio TS = %llu\n", (pkt.dts * atb) / (SEC2PICO / SEC2MILI));
			}
		}
		av_packet_unref(&pkt);
	} //While ends
  vidlen_total=(playtime_downloaded/1000)+1;
  //printf("Demuxing succeeded.\n");

  fclose(res_report);
  /* plooting results  */
  //FILE *pipe = popen("gnuplot -persist","w");
  //fprintf(pipe, "set title 'Video Playback Resolution Change Report'\n");
  //fprintf(pipe, "set xlabel 'Playback Timestamp'\n");
  //fprintf(pipe, "set ylabel 'Video Resolution'\n");
  //fprintf(pipe, "plot 'Res_Report.dat' using 1:2 with lines\n");
  //fprintf(pipe, "set term png\n");
  //fprintf(pipe, "set output \"res_report_plot.png\"\n");
  //fprintf(pipe, "replot\n");
  //fprintf(pipe, "set term x11\n");
  //pclose(pipe);

  avformat_close_input(&fmt_ctx);
  av_frame_free(&frame);
  fclose(fp);
  /* the timer will expire when whole video is played */
  printf("Waiting for the player to finish playing...\n");
  pthread_join(tids,NULL);

  return ret < 0;
}

void *ptimer(){
  struct timeval begin, now;
  gettimeofday(&begin, NULL);
  while (1){
    gettimeofday(&now, NULL);
    playtime_played = ((now.tv_sec - begin.tv_sec))*1000 + ((now.tv_usec - begin.tv_usec)/1000)+resume;
    if((playtime_played/1000)==vidlen_total){
      break;
    }
    if((playtime_diff<100 && playtime_played>0)|| (playtime_played>playtime_downloaded && vidlen_total==-1)){
      resume= playtime_played;
      struct timespec ts;
      ts.tv_sec = (playtime_diff / 1000)*-1;
      ts.tv_nsec = ((playtime_diff % 1000) * 1000000)*-1;
      nanosleep(&ts, NULL);
      memset(&begin, 0, sizeof(begin));
      gettimeofday(&begin, NULL);
    }
  }
  pthread_exit(0);
  return EXIT_SUCCESS;
}
