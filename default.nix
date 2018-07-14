with import "/home/jmsb/exps/nixpkgs" {};
let
in stdenv.mkDerivation rec {
  ffiPath = "${libffi.dev}/include";
  name = "sangha";

  buildInputs = [
    glew
    gst_all_1.gstreamer
    gst_all_1.gst-editing-services
    gst_all_1.gst-plugins-bad
    gst_all_1.gst-plugins-base
    gst_all_1.gst-libav
    xorg.libX11
    libGLU_combined
    pkgconfig
    glfw
  ];
shellHook = '' '';
}
