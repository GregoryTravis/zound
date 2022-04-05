%arg in inp score
%arg in inp0 mono
%arg in inp1 mono
%arg out outp mono

%lib inst

%sect global

double rat=1/(float)2;

%sect init

%sect loop

int i;

// Check the joystick
for (i=0; i<scorebuflen(inp); ++i) {
//  if (pitchwheelp(midibyte(inp,i,0))) {
  if (INST_ISPITCHWHEEL(scoreev(inp,i).inst)) {
    int value = scoreev(inp,i).value;
    rat = (float)value / (float)16384;
    printf( "%d %f\n", value, rat );
    break;
  }
}

// Generate the samples
for (i=0; i<BUFLEN; ++i) {
//  aout(outp,i) = (ain(inp0,i) + ain(inp1,i))>>1;
  aout(outp,i) = (ain(inp0,i)*rat + ain(inp1,i)*(1-rat));
}
