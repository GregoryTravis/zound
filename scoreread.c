%arg out outp score
%paramdefault readall 1
%paramdefault offset 0

%lib score io

%sect global

char *filename;
int fd, readallp;
score_event sev;
int sev_ready=0;
score_event *whole_score;
int whole_score_len;
int whole_score_p = 0;
int offset;

%sect init

offset = param_int( "offset" );
readallp = param_int( "readall" );
filename = param_string( "fn" );
fd = io_open( filename, IO_R );
if (readallp) {
  long l = io_length( fd );
  int r;
  whole_score_len = l / sizeof( score_event );
  if (whole_score_len*sizeof(score_event)!=l) {
    printf( "%s is not a whole # of events!\n", filename );
  }
  whole_score = (score_event*)malloc( sizeof(score_event) * whole_score_len );
  r = io_read( fd, whole_score, sizeof(score_event)*whole_score_len );
  if (r != sizeof(score_event)*whole_score_len) {
    fprintf( stderr, "Can't read %s fully!\n", filename );
    exit( 1 );
  }
  io_close( fd );
}

%sect loop

int en=0;
long midi_curtime = curtime - offset;
while (1) {
  if (sev_ready) {
    if (sev.time>midi_curtime) {
      //printf( "have but not ready %d < %d\n", sev.time, midi_curtime );
      break;
    } else {
      scoreev(outp,en) = sev;
      en++;
      sev_ready = 0;
    }
  } else {
    int r;
    if (readallp) {
      if (whole_score_p >= whole_score_len) {
	sev_ready = 0;
	DONE;
	break;
      } else {
        sev_ready = 1;
        sev = whole_score[whole_score_p++];
        //printf( "got event time %d\n", sev.time );
      }
    } else {
      r = score_read_event( fd, &sev, 1 );
      if (r<sizeof(sev)) {
	sev_ready = 0;
	DONE;
	break;
      }
      //printf( "read event time %d\n", sev.time );
      sev_ready = 1;
    }
  }
}
scorebuflen(outp) = en;
