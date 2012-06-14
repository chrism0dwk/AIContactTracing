INCLUDES = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/data
METASOURCES = AUTO
bin_PROGRAMS = epiMCMC
noinst_HEADERS = adaptive.h aiMCMC.h aifuncs.h
epiMCMC_SOURCES = adaptive.cpp aiMCMC.cpp aifuncs.cpp
epiMCMC_LDADD = $(top_builddir)/src/data/libepiData.la \
	$(top_builddir)/src/common/librandom.la -lm
