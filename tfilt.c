%arg in inp mono
%arg in ctrl score
%arg out outp mono
%arg param filter

%lib inst

%sect global

#include <stdio.h>

#define LEN 256

#define NTUNESTEPS 256
char buf[1024];
double a[NTUNESTEPS][LEN];
int na=0;
double b[NTUNESTEPS][LEN];
int nb=0;
double xhist[LEN];
int xhistp=0;
double yhist[LEN];
int yhistp=0;

int iir=0;
double alpha = 0.95;
double beta = 0.999;
double gain = 2;
double startbeta = 0.999;
double endbeta = -0.9;
double dbeta;
int tuning=NTUNESTEPS/2;

int i;
int nsamps=0;

%sect init

na = 3;
nb = 2;

dbeta = (endbeta - startbeta) / NTUNESTEPS;

for (i=0; i<NTUNESTEPS; ++i) {
  beta = startbeta + i*dbeta;
  a[i][0] = gain*((1-alpha)/2);
  a[i][1] = 0;
  a[i][2] = gain*(-(1-alpha)/2);
  b[i][0] = beta*(1+alpha);
  b[i][1] = -alpha;
  //printf( "%f %f %f %f %f\n", a[i][0], a[i][1], a[i][2], b[i][0], b[i][1] );
}

fprintf( stderr, "FP %d %d\n", na, nb );

%sect loop

int ii;
//int tuning = (int)( ((double)nsamps / 256000.0) * NTUNESTEPS );
//if (tuning >= NTUNESTEPS) tuning = NTUNESTEPS-1;

// Look for joystick input
for (i=0; i<scorebuflen(ctrl); ++i) {
  if (INST_ISPITCHWHEEL(scoreev(ctrl,i).inst)) {
    int value = scoreev(ctrl,i).value;
    tuning = (int)((value/16384.0)*NTUNESTEPS);
    if (tuning >= NTUNESTEPS) tuning = NTUNESTEPS-1;
    if (tuning < 0) tuning = 0;
    printf( "HEY %d %d\n", value, tuning );
  }
}

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
      acc += a[tuning][i] * xhist[hp];
      hp = (hp+na-1)%na;
    }
  }
  if (nb>0) {
    hp = (yhistp+nb-1) % nb;
    for (i=0; i<nb; ++i) {
      acc += b[tuning][i] * yhist[hp];
      hp = (hp+nb-1)%nb;
    }
  }
//  printf( "%f\n", acc );
  if (nb>0) {
    yhist[yhistp] = acc;
    yhistp = (yhistp+1) % nb;
  }
  ss = (int)(acc*32768);
  aout(outp,ii) = ss*10;
}
nsamps += BUFLEN;
