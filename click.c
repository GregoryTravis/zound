%arg out outp score

%lib score inst

%sect global

long lastbeat=-1;

%sect init

%sect loop

int b;
b = BEATNUM();
if (b!=lastbeat) {
  score_event sev;
  sev.inst = INST_NOTE(71);
  sev.pitch = 71;
  sev.vol = 87;
  sev.action = SCORE_NON;
  sev.midi_channel = 7;

  scoreev(outp,0) = sev;
  scorebuflen(outp) = 1;

  lastbeat = b;

  printf( "%d\n", BEATNUM() );
} else {
  scorebuflen(outp) = 0;
}
