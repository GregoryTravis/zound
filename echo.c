%arg in inp mono
%arg out outp mono

%sect global

sample *buf;
int echo_len;
int p;

%sect init

echo_len = param_int( "echolen" );
buf = (sample*)malloc( echo_len*sizeof(sample) );

%sect loop

int i;
for (i=0; i<BUFLEN; ++i) {
  aout(outp,i) = (buf[p] + ain(inp,i))/2;
  buf[p] = ain(inp,i);
  p++;
  if (p>=echo_len) p = 0;
}
