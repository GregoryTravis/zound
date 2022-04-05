%buffering 8 5
%node midiin outp=three fn:/dev/midi
%node read outp=one fn:$$1
%node tfilt inp=one outp=two ctrl=three filter:$$0
#%node write inp=two fn:$$2
%node spkr inp=two
