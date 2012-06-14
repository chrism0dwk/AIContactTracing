INCLUDES = -I$(top_srcdir)/src/data -I$(top_srcdir)/src/common
METASOURCES = AUTO
bin_PROGRAMS = aiGillespieSim

noinst_HEADERS = GillespieSim.hpp

aiGillespieSim_SOURCES = aiGillespieSim.cpp GillespieSim.cpp
aiGillespieSim_LDADD = $(top_builddir)/src/data/libepiData.la -lgsl -lgslcblas -lxerces-c -lboost_program_options

