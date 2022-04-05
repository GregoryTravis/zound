%node read outp=one fn:$$1
%node filt inp=one outp=two filter:$$0
%node write inp=two fn:$$2
