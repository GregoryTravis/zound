Zound: Fast patch-oriented audio synthesis in C
====

Zound is a dataflow audio synthesis engine written in C. Audio units are written
C with special annotations, and audio patches in a simple line-oriented graph
description language.

Audio unit:

```
%arg out outp mono
%arg param freq
%arg param len
%arg param phase

%sect global

#include <math.h>
int num_samples;
int yet=0;
int freq;
int len;
double phase;
double inc;

%sect init

num_samples = 0;
freq = param_int("freq");
len = param_int("len");
phase = param_double("phase");
inc = ((double)freq * 2 * 3.1415926535) / (double)SAMPRATE;
printf( "%d %d %f %f\n", freq, len, phase, inc );

%sect loop

int i;
for (i=0; i<BUFLEN; ++i) {
  double s = sin( (num_samples+i)*inc );
  aout(outp,i) = (sample)(32767 * s);
}
num_samples += BUFLEN;
if (num_samples>len) DONE;
```

Patch file:

```
%node sinwav outp=one freq:$$0 len:$$1 phase:0
%node write inp=one fn:$$2
```

