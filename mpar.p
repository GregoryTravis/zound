%node scoreread outp=one fn:$$0
%node scoredump inp=one
%node midiout inp=one fn:/dev/midi
%node mic outp=two
%node write inp=two fn:$$1
