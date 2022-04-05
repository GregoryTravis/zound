%buffering 8 5
%node midiin outp=one fn:/dev/midi
%node ss inp=one outp=two
%node spkr inp=two
