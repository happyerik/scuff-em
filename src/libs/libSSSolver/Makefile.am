noinst_LTLIBRARIES = libSSSolver.la
pkginclude_HEADERS = SSSolver.h

libSSSolver_la_SOURCES = 	\
 Excitations.cc    	\
 SSSolver.cc		\
 GetPPI.cc		\
 GetPhiE.cc		\
 PostProcessing.cc

AM_CPPFLAGS = -I$(top_srcdir)/src/libs/libhrutil    \
              -I$(top_srcdir)/src/libs/libhmat      \
              -I$(top_srcdir)/src/libs/libMatProp   \
              -I$(top_srcdir)/src/libs/libSpherical \
              -I$(top_srcdir)/src/libs/libSGJC      \
              -I$(top_srcdir)/src/libs/libTriInt    \
              -I$(top_srcdir)/src/libs/libMDInterp  \
              -I$(top_srcdir)/src/libs/libSubstrate \
              -I$(top_srcdir)/src/libs/libscuff     \
              -I$(top_srcdir)/src/libs/libIncField  \
              -I$(top_builddir) # for config.h
