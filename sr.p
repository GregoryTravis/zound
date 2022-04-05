%node midiin outp=one fn:/dev/midi
%node scoredump inp=one
%node scorewrite inp=one fn:$$0
