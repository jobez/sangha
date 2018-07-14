with import "/home/jmsb/exps/nixpkgs" {};
let
in stdenv.mkDerivation rec {
  ffiPath = "${libffi.dev}/include";
  name = "c-reload";

  buildInputs = [
    pkgconfig
  ];
shellHook = ''
export PATH=$PATH:/home/jmsb/exps/langs/lisp/common/compilec/c-mera:

'';
}
