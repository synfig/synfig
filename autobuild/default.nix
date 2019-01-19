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

*/

with import <nixpkgs> {};

let
  version = "git";
in
stdenv.mkDerivation rec {
  name = "synfigstudio-${version}";

  src = [ ../autobuild ../ETL  ../synfig-core ../synfig-studio ] ;

  nativeBuildInputs = [ pkgconfig autoreconfHook gettext gnumake makeWrapper ];
  buildInputs = [
    boost cairo ffmpeg fftw intltool glibmm gnome3.defaultIconTheme gtk3 gtkmm3
    imagemagick intltool libjpeg libjack2 libsigcxx libxmlxx mlt
    pango which
  ];

  preConfigure = "";
  configureScript = "";

  BOOST_CONFIGURE_OPTIONS = " --with-boost=${boost.dev} --with-boost-libdir=${boost.out}/lib ";
  ADWAITA_PATH = "${gnome3.adwaita-icon-theme.out}";
  MAKE_WRAPPER_PATH = "${makeWrapper.out}";

  sourceRoot = ".";

  postUnpack = ''
    touch configure.ac
  '';

  buildPhase = ''
    PREFIX=$out ./autobuild/build.sh
  '';

  installPhase = ''
    echo
  '';

  postInstall = ''
    wrapProgram "./_production/build/bin/synfigstudio" \
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
