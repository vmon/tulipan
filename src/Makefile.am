bin_PROGRAMS = tulipan
tulipan_SOURCES = tulipan.cpp c_l_jpeg_interface.cpp c_l_dct_coefficient.cpp \
				c_l_randomgenerator.cpp c_l_sharandomgenerator.cpp c_l_wetsteganographer.cpp \
				c_l_binarymatrix.cpp c_l_dynamicmatrix.cpp aBinarySequence.cpp SHA.cpp c_l_selectorruler.cpp \
		c_l_perturbedquantizer.cpp c_l_wetencoder.cpp c_l_wetdecoder.cpp l_tulipanglobal.cpp \
	c_l_randompool.cpp

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
tulipan_LDFLAGS = $(all_libraries) 
noinst_HEADERS = jpeglib.h c_l_jpeg_interface.h c_l_dct_coefficient.h \
				c_l_randomgenerator.h c_l_sharandomgenerator.h c_l_wetsteganographer.h c_l_binarymatrix.h \
				c_l_dynamicmatrix.h aBinarySequence.h SHA.h MessageDigest.h common.h c_l_selectorruler.h \
			c_l_perturbedquantizer.h c_l_wetencoder.h c_l_wetdecoder.h c_l_randompool.h jmorecfg.h jdct.h
tulipan_LDADD = libjpeg-tlp.a
