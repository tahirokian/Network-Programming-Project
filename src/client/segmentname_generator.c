#include <stdio.h>
#include <string.h>
#include "client.h"

char *replace_str(char *str, char *orig, char *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

char *segNameGen(char *media,char *bandwidth, int segnum)
{
  char* draft1;
  char* draft2;

  char segnum_str[5];
  sprintf(segnum_str, "%d", segnum ); /* convert int to string */

  draft1=replace_str(media, "$Bandwidth$", bandwidth); /* first replace $Bandwidth$ */
  draft2=replace_str(draft1, "$Number$", segnum_str);	/* then replace $Number$ */

  return draft2;
}
