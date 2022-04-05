# Play to the speaker in stereo

%node read outp=one fn:$$0
%node read outp=two fn:$$1
%node sspkr inp0=one inp1=two
