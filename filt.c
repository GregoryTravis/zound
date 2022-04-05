%arg in inp mono
%arg out outp mono
%arg param filter

%sect global

#include <stdio.h>

#define LEN 256

char buf[1024];
double a[LEN];
int na=0;
double b[LEN];
int nb=0;
double xhist[LEN];
int xhistp=0;
double yhist[LEN];
int yhistp=0;

char *def;
FILE *fp;
int iir=0;

%sect init

def = param_string( "filter" );

fp = fopen( def, "r" );
if (!fp) { fprintf( stderr, "Ha!\n" ); exit( 1 ); }

while (fgets( buf, 1024, fp ) ) {
  if (buf[0]==10) {
    iir=1;
    continue;
  }
  if (iir)
    b[nb++] = atof( buf );
  else
    a[na++] = atof( buf );
}
fprintf( stderr, "FP %d %d\n", na, nb );

%sect loop

int i, ii;
for (ii=0; ii<BUFLEN; ++ii) {
  sample s = ain(inp,ii), ss;
  int hp;
  float v = ((float)s)/32768;

  double acc=0;

  if (na>0) {
    xhist[xhistp] = v;
    xhistp = (xhistp+1) % na;
  }
  if (na>0) {
    hp = (xhistp+na-1) % na;
    for (i=0; i<na; ++i) {
      acc += a[i] * xhist[hp];
      hp = (hp+na-1)%na;
    }
  }
  if (nb>0) {
    hp = (yhistp+nb-1) % nb;
    for (i=0; i<nb; ++i) {
      acc += b[i] * yhist[hp];
      hp = (hp+nb-1)%nb;
    }
  }
//  printf( "%f\n", acc );
  if (nb>0) {
    yhist[yhistp] = acc;
    yhistp = (yhistp+1) % nb;
  }
  ss = (int)(acc*32768);
  aout(outp,ii) = ss;
}
