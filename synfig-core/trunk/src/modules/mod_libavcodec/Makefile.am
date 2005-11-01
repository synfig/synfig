# $Header: /opt/voria/cvs/synfig/src/modules/mod_libavcodec/Makefile.am,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/modules/mod_libavcodec/libavcodec -I$(top_srcdir)/src/modules/mod_libavcodec/libavformat

moduledir=@MODULE_DIR@

if WITH_LIBAVCODEC
module_LTLIBRARIES = libmod_libavcodec.la

# Add the filenames to the following line:
libmod_libavcodec_la_SOURCES = main.cpp trgt_av.cpp trgt_av.h mptr.cpp mptr.h

libmod_libavcodec_la_LDFLAGS = -module -no-undefined -lavcodec -lavformat
libmod_libavcodec_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_libavcodec_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@
else
endif
EXTRA_DIST= mod_libavcodec.nsh unmod_libavcodec.nsh libavcodec/ac3.h libavcodec/ac3tab.h libavcodec/avcodec.h libavcodec/bswap.h libavcodec/cabac.h libavcodec/common.h libavcodec/dsputil.h libavcodec/dvdata.h libavcodec/fastmemcpy.h libavcodec/golomb.h libavcodec/h263data.h libavcodec/h264data.h libavcodec/i386/dsputil_mmx_avg.h libavcodec/i386/dsputil_mmx_rnd.h libavcodec/i386/mmx.h libavcodec/imgconvert_template.h libavcodec/indeo3data.h libavcodec/liba52/a52.h libavcodec/liba52/a52_internal.h libavcodec/liba52/a52_util.h libavcodec/liba52/bitstream.h libavcodec/liba52/mm_accel.h libavcodec/liba52/tables.h libavcodec/libpostproc/mangle.h libavcodec/libpostproc/postprocess.h libavcodec/libpostproc/postprocess_internal.h libavcodec/mpeg12data.h libavcodec/mpeg4data.h libavcodec/mpegaudio.h libavcodec/mpegaudiodectab.h libavcodec/mpegaudiotab.h libavcodec/mpegvideo.h libavcodec/msmpeg4data.h libavcodec/oggvorbis.h libavcodec/ra144.h libavcodec/ra288.h libavcodec/simple_idct.h libavcodec/svq1_cb.h libavcodec/svq1_vlc.h libavcodec/vp3data.h libavcodec/wmadata.h libavformat/avformat.h libavformat/avi.h libavformat/avio.h libavformat/barpainet.h libavformat/dv1394.h libavformat/dvcore.h libavformat/framehook.h libavformat/mpegts.h libavformat/os_support.h libavformat/rtp.h libavformat/rtsp.h libavformat/rtspcodes.h
