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

with import <nixpkgs> {};

let
  version = "git";

  ETL = stdenv.mkDerivation rec {
    name = "ETL-${version}";

    src = ../ETL;

    nativeBuildInputs = [ autoreconfHook ];
  };

  synfig = stdenv.mkDerivation rec {
    name = "synfig-${version}";

    src = ../synfig-core;

    configureFlags = [
      "--with-boost=${boost.dev}"
      "--with-boost-libdir=${boost.out}/lib"
    ];

    nativeBuildInputs = [ pkgconfig autoreconfHook gettext ];

    buildInputs = [
      ETL boost cairo fftw glibmm intltool libjpeg libsigcxx libxmlxx
      mlt imagemagick pango which
    ];

    preConfigure = "./bootstrap.sh";

  };
in
stdenv.mkDerivation rec {
  name = "synfigstudio-${version}";

  src = ../synfig-studio;

  preConfigure = "./bootstrap.sh";

  nativeBuildInputs = [ pkgconfig autoreconfHook gettext ];
  buildInputs = [
    ETL boost cairo fftw glibmm gnome3.defaultIconTheme gtk3 gtkmm3
    imagemagick intltool libjack2 libsigcxx libxmlxx makeWrapper mlt
    synfig which
  ];

  postInstall = ''
    wrapProgram "$out/bin/synfigstudio" \
      --prefix XDG_DATA_DIRS : "$XDG_ICON_DIRS:$GSETTINGS_SCHEMAS_PATH" \
      --prefix XCURSOR_PATH : "${gnome3.adwaita-icon-theme.out}/share/icons" \
      --set XCURSOR_THEME "Adwaita"
  '';

  enableParallelBuilding = true;

  meta = with stdenv.lib; {
    description = "A 2D animation program";
    homepage = http://www.synfig.org;
    license = licenses.gpl2Plus;
    maintainers = [ maintainers.goibhniu ];
    platforms = platforms.linux;
  };
}
