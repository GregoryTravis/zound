%arg in inp0 score
%arg in inp1 score
%arg out outp score

%sect global

%sect init

%sect loop

int i0=0, i1=0, o=0;
int cd0 = scorebuflen(inp0), cd1 = scorebuflen(inp1);

//if (cd0 || cd1) printf( "smix %d %d\n", cd0, cd1 );

while (cd0>0 && cd1>0) {
  if (scoreev(inp0,i0).time < scoreev(inp1,i1).time) {
    scoreev(outp,o) = scoreev(inp0,i0);
    i0++;
    cd0--;
  } else {
    scoreev(outp,o) = scoreev(inp1,i1);
    i1++;
    cd1--;
  }
  o++;
}

while (cd0>0) {
  scoreev(outp,o) = scoreev(inp0,i0);
  o++;
  i0++;
  cd0--;
}

while (cd1>0) {
  scoreev(outp,o) = scoreev(inp1,i1);
  o++;
  i1++;
  cd1--;
}

scorebuflen(outp) = o;
//if (o) printf( "  smix %d\n", o );
