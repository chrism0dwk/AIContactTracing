INCLUDES = -I$(top_srcdir)/src/common
METASOURCES = AUTO
noinst_LTLIBRARIES = libepiData.la
noinst_HEADERS = SAXContactParse.hpp XmlCTWriter.hpp configExceptions.h \
	contactMatrix.h contactTrace.hpp epiconfig.h infection.hpp occultReader.h \
	occultWriter.h posterior.h sinrEpi.h sinrParms.h sparseMatrix.h speciesMat.h
libepiData_la_SOURCES = SAXContactParse.cpp XmlCTWriter.cpp \
	configExceptions.cpp contactMatrix.cpp contactTrace.cpp epiconfig.cpp infection.cpp \
	occultReader.cpp occultWriter.cpp posterior.cpp sinrEpi.cpp sparseMatrix.cpp \
	speciesMat.cpp
libepiData_la_LIBADD = $(top_builddir)/src/common/libstlStrTok.la -lm
SUBDIRS = config
