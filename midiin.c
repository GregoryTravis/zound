%rem read from midi device

%arg out outp score

%lib score midi

%sect global

char *filename;

%sect init

filename = param_string( "fn" );
midi_open( filename );

%sect loop

int i=0;
while (1) {
  score_event sev;
  int more, done;
  midi_read_event( &sev, &more, &done );
  if (!more) break;
  memcpy( &scoreev(outp,i), &sev, sizeof(sev) );
  scoreev(outp,i).time = curtime;
  i++;
}
scorebuflen(outp) = i;
if (midi_eof()) DONE;
