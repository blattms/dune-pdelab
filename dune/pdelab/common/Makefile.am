SUBDIRS =                                       \
	typetree

commondir = $(includedir)/dune/pdelab/common
common_HEADERS =				\
        benchmarkhelper.hh                      \
	clock.hh				\
	countingptr.hh				\
	cpstoragepolicy.hh			\
	crossproduct.hh				\
	function.hh				\
	functionutilities.hh			\
	functionwrappers.hh			\
	geometrywrapper.hh			\
	hostname.hh				\
	jacobiantocurl.hh			\
	logtag.hh				\
	multiindex.hh				\
	range.hh				\
	topologyutility.hh			\
	typetree.hh                             \
        typetraits.hh                           \
	vtkexport.hh

noinst_LTLIBRARIES = libpdelabcommon.la
libpdelabcommon_la_SOURCES =			\
	clock.cc				\
	hostname.cc				\
	logtag.cc
libpdelabcommon_la_CPPFLAGS = $(AM_CPPFLAGS)	\
	$(POSIX_CLOCK_CPPFLAGS)
libpdelabcommon_la_LDFLAGS = $(AM_LDFLAGS)	\
	$(POSIX_CLOCK_LDFLAGS)
libpdelabcommon_la_LIBADD =			\
	typetree/libpdelabtypetree.la		\
	$(POSIX_CLOCK_LIBS)

include $(top_srcdir)/am/global-rules
