#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>
#include "z_time.h"

void main()
{
  short s = -32768;
  printf( "%d\n", ((unsigned short)s)>>8 );
}
