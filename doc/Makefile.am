# $Id: Makefile.am 3930 2007-12-05 17:17:07Z robertk $

SUBDIRS = doxygen

# setting like in dune-web
BASEDIR=..
CURDIR=doc

if ! BUILD_DOCS
# add tag to notify that dist has been build without documentation 
dist-hook:
	echo "# No documentation included in distribution! " > $(distdir)/$(DOCUMENTATION_TAG_FILE) 
endif

include $(top_srcdir)/am/webstuff
include $(top_srcdir)/am/global-rules
