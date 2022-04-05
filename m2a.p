# render midi to audiofile using external device
%node scoreread outp=one fn:$$0
%node midiout inp=one fn:/dev/midi
%node mic outp=two
%node write inp=two fn:$$1
