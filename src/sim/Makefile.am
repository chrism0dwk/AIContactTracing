INCLUDES = -I$(top_srcdir)/src/data -I$(top_srcdir)/src/common
METASOURCES = AUTO
bin_PROGRAMS = sellkeSim simContacts simCTEpidemic

noinst_HEADERS = aiModel.hpp Model.hpp AIPopulation.hpp Population.hpp Parameter.hpp EventQueue.hpp EventParser.hpp SimOnContact.hpp Individual.hpp

sellkeSim_SOURCES = main.cpp AIPopulation.cpp aiModel.cpp Individual.cpp
sellkeSim_LDADD = $(top_builddir)/src/data/libepiData.la -lgsl -lgslcblas

simContacts_SOURCES = simContacts.cpp AIPopulation.cpp aiModel.cpp Individual.cpp Parameter.cpp
simContacts_LDADD = $(top_builddir)/src/data/libepiData.la -lgsl -lgslcblas -lxerces-c

simCTEpidemic_SOURCES = simCTEpidemic.cpp SimOnContact.cpp EventQueue.cpp EventParser.cpp Individual.cpp
simCTEpidemic_LDADD = $(top_builddir)/src/data/libepiData.la -lxerces-c 

SUBDIRS = gillespie
