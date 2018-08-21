with import "/home/jmsb/exps/nixpkgs" {};
let
in stdenv.mkDerivation rec {
  ffiPath = "${libffi.dev}/include";
  name = "sangha";

  GST_PLUGIN_PATH = lib.makeSearchPathOutput "lib" "lib/gstreamer-1.0" [
    gst_all_1.gst-editing-services
    gst_all_1.gst-plugins-base
    gst_all_1.gst-plugins-good
    gst_all_1.gst-plugins-bad
    gst_all_1.gst-plugins-ugly
    gst_all_1.gst-libav
    gst-ffmpeg

  ];

  LD_LIBRARY_PATH = stdenv.lib.makeLibraryPath [
    gst_all_1.gst-editing-services
    gst_all_1.gst-plugins-base
    gst_all_1.gst-plugins-good
    gst_all_1.gst-plugins-bad
    gst_all_1.gst-plugins-ugly
    gst_all_1.gst-libav
    gst-ffmpeg

  ];

  buildInputs = [
    glew
    gst_all_1.gstreamer
    gst_all_1.gst-editing-services
    gst_all_1.gst-plugins-bad
    gst_all_1.gst-plugins-base
    gst_all_1.gst-libav
    xorg.libX11
    libpng
    freeglut
    libGLU_combined
    pkgconfig
    glfw
    llvm_5

    jack2Full
    faust2
    fftw
    aubio
    glm
    asio
    gperftools
    libsndfile

  ];
  shellHook = ''
export ASIO_PATH=${asio};
'';
}
