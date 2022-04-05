%node read outp=one fn:$$0
%node read outp=two fn:$$1
%node mix2 inp0=one inp1=two outp=three
%node write inp=three fn:$$2
