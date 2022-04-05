%buffering 4 5
%node midiin outp=midi fn:/dev/midi
%node read outp=one fn:$$0
%node read outp=two fn:$$1
%node jmix2 inp0=one inp1=two inp=midi outp=three
%node spkr inp=three
#%node write inp=three fn:$$2
