%node midiin outp=one fn:/dev/midi
%node scorewrite inp=one fn:$$1
%node scoreread outp=two fn:$$0
%node midiout inp=two fn:/dev/midi
%node scoredump inp=one
