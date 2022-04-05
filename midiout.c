%rem write to midi device

%arg in inp score

%lib score midi

%sect global

char *filename;

%sect init

filename = param_string( "fn" );
midi_open( filename );

%sect loop

// We're just going to assume all events are timely

int i;
for (i=0; i<scorebuflen(inp); ++i) {
  midi_write_event( &scoreev(inp,i) );
}
