%arg in inp mono

%sect global

sample total;

%sect init

total = 0;

%sect loop

{
int i;
for (i=0; i<BUFLEN; ++i) {
  total += ain(inp,i);
}
}
//if (yet++>4) DONE;
