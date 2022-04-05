%arg in inp score
%arg out outp mono

%lib sslib soundio inst

%sect global

%sect init

ss_init();

%sect loop

int i;

for (i=0; i<scorebuflen(inp); ++i) {
  if (INST_ISNOTE(scoreev(inp,i).inst) && scoreev(inp,i).action==SCORE_NON) {
    ss_note_on( curtime, 60 );
  }
}

ss_render( rawbuf(outp), BUFLEN );
