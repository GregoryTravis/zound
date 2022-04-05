%arg out outp mono

%sect global

sample last;
int yet=0;

%sect init

last = 0;

%sect loop

{
int i;
for (i=0; i<BUFLEN; ++i) {
  aout(outp,i) = 0;
  last++;
}
}
if (yet++>2) DONE;
