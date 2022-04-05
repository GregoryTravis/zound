%node scoreread outp=one fn:$$0 offset:$$2
%node read outp=two fn:$$1
%node spkr inp=two
%node midiout inp=one fn:/dev/midi
