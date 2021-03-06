BUILT_SOURCES = rcsim.h
rcsim.h:
	pushd $(top_srcdir)/src/ && ./create_rcsim_h.sh > $@

.PHONY: rcsim.h

AM_CXXFLAGS = -fno-rtti -fno-exceptions -D__STDC_FORMAT_MACROS

SUBDIRS = zapit gui daemonc driver system eitd timerd

if ENABLE_WEBIF
SUBDIRS += nhttpd
endif

AM_CPPFLAGS = \
	-I$(srcdir) \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/src/zapit/include \
	-I$(top_srcdir)/lib/connection \
	-I$(top_srcdir)/lib/libeventserver \
	-I$(top_srcdir)/lib/libconfigfile \
	-I$(top_srcdir)/lib/libnet \
	-I$(top_srcdir)/lib/xmltree \
	-I$(top_srcdir)/lib/libmd5sum \
	-I$(top_srcdir)/lib/libupnpclient \
	@FREETYPE_CFLAGS@ \
	@LIBCS_CFLAGS@ \
	@SIGC_CFLAGS@

if BOXTYPE_COOL
if BOXMODEL_APOLLO
AM_CPPFLAGS += -I$(top_srcdir)/lib/libcoolstream2
else
AM_CPPFLAGS += -I$(top_srcdir)/lib/libcoolstream
endif
endif
if USE_STB_HAL
AM_CPPFLAGS += -I$(STB_HAL_INC)
endif
if BOXTYPE_TRIPLE
SUBDIRS += lcddisplay
endif

if USE_TREMOR
VORBISLIBS = @VORBISIDEC_LIBS@
else
VORBISLIBS = -lvorbisfile -lvorbis -logg
endif

if ENABLE_FLAC
FLACLIBS = -lFLAC
AM_CPPFLAGS += -DENABLE_FLAC
else
FLACLIBS =
endif

if ENABLE_FFMPEGDEC
ADEC_LIBS = @SWRESAMPLE_LIBS@
else
ADEC_LIBS = @MAD_LIBS@ \
	@ID3TAG_LIBS@ \
	$(FLACLIBS) \
	$(VORBISLIBS)
endif

bin_PROGRAMS = neutrino rcsim

neutrino_SOURCES = neutrino_menue.cpp neutrino.cpp

AM_CPPFLAGS += -D_FILE_OFFSET_BITS=64

if BOXMODEL_APOLLO
MTDUTILSLIBS = \
	system/mtdutils/libneutrino_system_mtdutils.a \
	system/mtdutils/lib/libneutrino_system_mtdutils_lib.a
endif

neutrino_LDADD = \
	daemonc/libneutrino_daemonc.a \
	gui/bedit/libneutrino_gui_bedit.a \
	gui/libtimerlist.a \
	gui/libneutrino_gui.a \
	gui/widget/libneutrino_gui_widget.a \
	gui/widget/libneutrino_gui_widget2.a \
	driver/pictureviewer/libneutrino_pictureviewer.a \
	system/libneutrino_system.a \
	$(MTDUTILSLIBS) \
	gui/movieinfo.o \
	gui/libneutrino_gui2.a \
	gui/components/libneutrino_gui_components.a \
	eitd/libsectionsd.a \
	gui/volumebar.o \
	driver/libneutrino_driver.a \
	driver/libneutrino_driver_netfile.a \
	timerd/libtimerd.a \
	zapit/src/libzapit.a \
	$(top_builddir)/lib/sectionsdclient/libsectionsdclient.a \
	$(top_builddir)/lib/timerdclient/libtimerdclient.a \
	$(top_builddir)/src/zapit/lib/libzapitclient.a \
	$(top_builddir)/lib/libconfigfile/libtuxbox-configfile.a \
	$(top_builddir)/lib/connection/libtuxbox-connection.a \
	$(top_builddir)/lib/libeventserver/libtuxbox-eventserver.a \
	$(top_builddir)/lib/xmltree/libtuxbox-xmltree.a \
	$(top_builddir)/lib/libnet/libtuxbox-net.a \
	$(top_builddir)/lib/libmd5sum/libtuxbox-md5sum.a \
	$(top_builddir)/lib/libtuxtxt/libtuxtxt.a \
	$(top_builddir)/lib/libdvbsub/libdvbsub.a \
	@CURL_LIBS@ \
	@FREETYPE_LIBS@ \
	@PNG_LIBS@ \
	@LIBCS_LIBS@ \
	$(ADEC_LIBS) \
	@SIGC_LIBS@ \
	-ldvbsi++ \
	-ljpeg \
	-lOpenThreads \
	-lrt -lpthread -lblkid

if ENABLE_WEBIF
neutrino_LDADD += \
	nhttpd/libnhttpd.a \
	nhttpd/tuxboxapi/coolstream/libnhttpd_tuxboxapi.a \
	nhttpd/yhttpd_mods/libyhttpdmods.a \
	nhttpd/yhttpd_core/libyhttpd.a
endif
if ENABLE_GRAPHLCD
neutrino_LDADD += -lglcddrivers -lglcdgraphics -lusb-1.0
endif
if ENABLE_GIFLIB
neutrino_LDADD += -lgif
else
neutrino_LDADD += -lungif
endif
if ENABLE_LUA
neutrino_LDADD += @LUA_LIBS@
endif

if ENABLE_UPNP
neutrino_LDADD += \
	$(top_builddir)/lib/libupnpclient/libtuxbox-upnpclient.a
endif

if BOXTYPE_COOL
neutrino_LDADD += \
	$(top_builddir)/lib/libcoolstream/libhwcaps.a
neutrino_LDADD += -lcoolstream-mt -lca-sc
if ENABLE_TMSDK 
else
if BOXMODEL_APOLLO
neutrino_LDADD += -lca-ci -llnxtmvssUsr -llnxUKAL -llnxplatUsr -llnxtmasUsr -llnxdvbciUsr -llnxpvrUsr -llnxcssUsr -llnxnotifyqUsr -ltmpvrDataManager-cst -ltmpvrIndexStorage -llnxscsUsr
else
neutrino_LDADD += -lnxp
endif
endif

if BOXMODEL_APOLLO
neutrino_LDADD += -liconv
endif

bin_PROGRAMS += drivertool
drivertool_SOURCES = drivertool.c
bin_PROGRAMS += dt
dt_SOURCES = dt.c
bin_PROGRAMS += rcsim
rcsim_SOURCES = rcsim.c rcsim.h
bin_PROGRAMS += mdev_helper
mdev_helper_SOURCES = mdev_helper.cpp

endif
if USE_STB_HAL
bin_PROGRAMS += rcsim
rcsim_SOURCES = rcsim.c rcsim.h

neutrino_LDADD += \
	-lstb-hal
neutrino_LDFLAGS = \
	$(STB_HAL_LIB)
endif
if BOXTYPE_TRIPLE
neutrino_LDADD += \
	$(top_builddir)/src/lcddisplay/liblcddisplay.a
endif

neutrino_LDADD += \
	@AVFORMAT_LIBS@ \
	@AVUTIL_LIBS@ \
	@AVCODEC_LIBS@ \
	@SWRESAMPLE_LIBS@
