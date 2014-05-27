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
      ETL autoconf automake boost cairo gettext glibmm libsigcxx
      libtool libxmlxx pango pkgconfig
    ];

    preConfigure = ''
      libtoolize --copy
      chmod +w libltdl/configure
      autoreconf --install
    '';

    configureFlags = with pkgs; [ "--with-boost-libdir=${boost}/lib" ];
  };
in 
stdenv.mkDerivation rec {
  name = "synfig-studio-git";

  src = ../synfig-studio;

  buildInputs = with pkgs; [
    ETL autoconf automake boost cairo gettext glibmm gtk gtkmm
    imagemagick intltool intltool jackaudio libsigcxx libtool libxmlxx
    pkgconfig synfig which
  ];

  preConfigure = "./bootstrap.sh";
}
