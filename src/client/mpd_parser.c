#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

Mpd * mpdParser(char * fileName, int * numMpdEntries){
   char *str_search="bandwidth=";	/* string comparison holder */
   char *str_duration="duration=";	/* string comparison holder */
   char *str_media="media=";		/* string comparison holder */

   static Mpd mpd_datas[100];		/* parsed mpd data holder */
   char temp[512];
   FILE * fp;
   int i=1;
   int num_mpd=0;
   fp = fopen (fileName, "rw+");

   rewind(fp);

   while(fgets(temp, 512, fp) != NULL) {
       if((strstr(temp, str_duration)) != NULL) {
           char* duration_str __attribute__ ((unused));
           char* duration __attribute__ ((unused));

           duration_str = strtok (temp,"\"");
           duration= strtok (NULL, "\"");
           
           strcpy(mpd_datas[0].duration,duration);
           break;
       }
   }

   rewind(fp);

   while(fgets(temp, 512, fp) != NULL) {
       if((strstr(temp, str_media)) != NULL) {
           char* media_str __attribute__ ((unused));
           char* media __attribute__ ((unused));
           char* initialization_str __attribute__ ((unused));
           char* initialization __attribute__ ((unused));
           char* text1 __attribute__ ((unused));
           char* text2 __attribute__ ((unused));
           char* text3 __attribute__ ((unused));
           char* text4 __attribute__ ((unused));
           char* text5 __attribute__ ((unused));
           char* text6 __attribute__ ((unused));

           text1 = strtok (temp,"\"");
           text2 = strtok (NULL,"\"");
           media_str = strtok (NULL,"\"");
           media= strtok (NULL, "\"");
           text3 = strtok (NULL,"\"");
           text4 = strtok (NULL,"\"");
           text5 = strtok (NULL,"\"");
           text6 = strtok (NULL,"\"");
           initialization_str = strtok (NULL,"\"");
           initialization= strtok (NULL, "\"");

           strcpy(mpd_datas[0].media,media);
           strcpy(mpd_datas[0].initialization,initialization);
           break;
       }
   }

   rewind(fp);

   while(fgets(temp, 512, fp) != NULL) {
       if((strstr(temp, str_search)) != NULL) {
           //printf("%s\n",temp);
            char* text __attribute__ ((unused));
            char* id_str __attribute__ ((unused));
            char* id __attribute__ ((unused));
            char* mimeType_str __attribute__ ((unused));
            char* mimeType __attribute__ ((unused));
            char* codecs_str __attribute__ ((unused));
            char* codecs __attribute__ ((unused));
            char* width_str __attribute__ ((unused));
            char* width __attribute__ ((unused));
            char* height_str __attribute__ ((unused));
            char* height __attribute__ ((unused));
            char* frameRate_str __attribute__ ((unused));
            char* frameRate __attribute__ ((unused));
            char* sar_str __attribute__ ((unused));
            char* sar __attribute__ ((unused));
            char* startWithSAP_str __attribute__ ((unused));
            char* startWithSAP __attribute__ ((unused));
            char* bandwidth_str __attribute__ ((unused));
            char* bandwidth __attribute__ ((unused));

            text = strtok (temp,"\"");
            id = strtok (NULL,"\"");
            mimeType_str = strtok (NULL, "\"");
            mimeType = strtok (NULL, "\"");
            codecs_str = strtok (NULL, "\"");
            codecs = strtok (NULL, "\"");
            width_str = strtok (NULL, "\"");
            width = strtok (NULL, "\"");
            height_str = strtok (NULL, "\"");
            height = strtok (NULL, "\"");
            frameRate_str = strtok (NULL, "\"");
            frameRate = strtok (NULL, "\"");
            sar_str = strtok (NULL, "\"");
            sar = strtok (NULL, "\"");
            startWithSAP_str = strtok (NULL, "\"");
            startWithSAP = strtok (NULL, "\"");
            bandwidth_str = strtok (NULL, "\"");
            bandwidth = strtok (NULL, "\"");

            strcpy(mpd_datas[i].id,id);
            strcpy(mpd_datas[i].mimeType,mimeType);
            strcpy(mpd_datas[i].codecs,codecs);
            strcpy(mpd_datas[i].width,width);
            strcpy(mpd_datas[i].height,height);
            strcpy(mpd_datas[i].frameRate,frameRate);
            strcpy(mpd_datas[i].sar,sar);
            strcpy(mpd_datas[i].startWithSAP,startWithSAP);
            strcpy(mpd_datas[i].bandwidth,bandwidth);
            i++;
        }
   }
   num_mpd= i--;
   *numMpdEntries = num_mpd;
   fclose(fp);
   return mpd_datas;
}
