INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_ADSB_SEND adsb_send)

FIND_PATH(
    ADSB_SEND_INCLUDE_DIRS
    NAMES adsb_send/api.h
    HINTS $ENV{ADSB_SEND_DIR}/include
        ${PC_ADSB_SEND_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    ADSB_SEND_LIBRARIES
    NAMES gnuradio-adsb_send
    HINTS $ENV{ADSB_SEND_DIR}/lib
        ${PC_ADSB_SEND_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ADSB_SEND DEFAULT_MSG ADSB_SEND_LIBRARIES ADSB_SEND_INCLUDE_DIRS)
MARK_AS_ADVANCED(ADSB_SEND_LIBRARIES ADSB_SEND_INCLUDE_DIRS)

