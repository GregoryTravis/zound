#include <stdio.h>

void main()
{
  short s;
  while (read( 0, &s, 2 )==2) {
    write( 1, &s, 2 );
    write( 1, &s, 2 );
    write( 1, &s, 2 );
    write( 1, &s, 2 );
    write( 1, &s, 2 );
    write( 1, &s, 2 );
  }
}
