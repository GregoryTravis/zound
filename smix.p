%option nonrealtime
%node scoreread outp=one fn:$$0
%node scoreread outp=two fn:$$1
%node smix inp0=one inp1=two outp=three
%node scorewrite inp=three fn:$$2
