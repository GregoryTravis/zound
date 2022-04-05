%arg in inp score

%lib score

%sect global

%sect init

%sect loop

int i;

for (i=0; i<scorebuflen(inp); ++i) {
  score_dump_event( &scoreev(inp,i) );
}
