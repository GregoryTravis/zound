%arg in inp0 mono
%arg in inp1 mono
%arg out outp mono

%sect global

%sect init

%sect loop

int i;
for (i=0; i<BUFLEN; ++i) {
  aout(outp,i) = (ain(inp0,i) + ain(inp1,i))>>1;
}
