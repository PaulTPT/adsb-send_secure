# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
list(APPEND adsb_send_sources
    adsbgen_impl.cc )

set(adsb_send_sources "${adsb_send_sources}" crypto.cpp crypto.h)
if(NOT adsb_send_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT adsb_send_sources)

add_library(gnuradio-adsb_send SHARED ${adsb_send_sources} crypto.cpp crypto.h)
target_link_libraries(gnuradio-adsb_send ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES})
set_target_properties(gnuradio-adsb_send PROPERTIES DEFINE_SYMBOL "gnuradio_adsb_send_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-adsb_send PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Setup OpenSSL
########################################################################

find_package (OpenSSL)
if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries (gnuradio-adsb_send ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-adsb_send
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_adsb_send_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_adsb_send.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_adsb_send.cc
)

add_executable(test-adsb_send ${test_adsb_send_sources} crypto.cpp crypto.h)

target_link_libraries(
  test-adsb_send
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  gnuradio-adsb_send
)

GR_ADD_TEST(test_adsb_send test-adsb_send)
