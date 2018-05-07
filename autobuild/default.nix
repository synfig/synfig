/*
This is a nix expression to build Synfig from source on any distro
where nix is installed. This will install all the dependencies from
the nixpkgs repo and build Synfig without interfering with the host
distro.

http://nixos.org/nix/

To quickly install nix, you can run the following command:

$ curl -L http://git.io/nix-install.sh | bash

To initialise it:

$ source ~/.nix-profile/etc/profile.d/nix.sh

To build synfig, from the current directory:

$ nix-build

To run the newly compiled synfigstudio:

$ ./result/bin/synfigstudio
*/

let
  pkgs = import <nixpkgs> {};
  stdenv = pkgs.stdenv; 

  ETL = stdenv.mkDerivation {
    name = "ETL-git";

    src = ../ETL;

    preConfigure = "autoreconf --install --force";

    buildInputs = with pkgs; [ autoconf automake ];
  };

  synfig = stdenv.mkDerivation  {
    name = "synfig-git";

    src = ../synfig-core;

    buildInputs = with pkgs; [
      ETL autoconf automake boost cairo fftw gettext glibmm intltool libsigcxx
      libtool libxmlxx mlt pango pkgconfig which
    ];

    preConfigure = "./bootstrap.sh";

    configureFlags = with pkgs; [ "--with-boost-libdir=${boost}/lib" ];
  };
in 
stdenv.mkDerivation rec {
  name = "synfig-studio-git";

  src = ../synfig-studio;

  buildInputs = with pkgs; [
    ETL autoconf automake boost cairo fftw gettext glibmm gtk3 gtkmm3
    imagemagick intltool intltool libjack2 libsigcxx libtool libxmlxx mlt
    pkgconfig synfig which
  ];

  preConfigure = "./bootstrap.sh";
}
