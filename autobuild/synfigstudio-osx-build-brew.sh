brew install autoconf automake libtool intltool gettext pkg-config glibmm libxml++ cairo fftw pango mlt boost zlib \
gtkmm3

# libtool for synfig-core glibtoolize
# gettext for autopoint

# autopoint is not in PATH after install (conflicting with system gettext https://github.com/Homebrew/legacy-homebrew/issues/24070)
PATH=/usr/local/opt/gettext/bin:$PATH ./bootstrap.sh
PATH=/usr/local/opt/gettext/bin:$PATH ./configure

#For compilers to find this software you may need to set:
#    LDFLAGS:  -L/usr/local/opt/libffi/lib
#For pkg-config to find this software you may need to set:
#    PKG_CONFIG_PATH: /usr/local/opt/libffi/lib/pkgconfig


#If you need to have this software first in your PATH run:
#  echo 'export PATH="/usr/local/opt/icu4c/bin:$PATH"' >> ~/.bash_profile
#  echo 'export PATH="/usr/local/opt/icu4c/sbin:$PATH"' >> ~/.bash_profile

#For compilers to find this software you may need to set:
#    LDFLAGS:  -L/usr/local/opt/icu4c/lib
#    CPPFLAGS: -I/usr/local/opt/icu4c/include
#For pkg-config to find this software you may need to set:
#    PKG_CONFIG_PATH: /usr/local/opt/icu4c/lib/pkgconfig

#For compilers to find this software you may need to set:
#    LDFLAGS:  -L/usr/local/opt/zlib/lib
#    CPPFLAGS: -I/usr/local/opt/zlib/include
#For pkg-config to find this software you may need to set:
#    PKG_CONFIG_PATH: /usr/local/opt/zlib/lib/pkgconfig

# zlib not checked