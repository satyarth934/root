############################################################################
# CMakeLists.txt file for building ROOT sql/odbc package
############################################################################

include_directories(${ODBC_INCLUDE_DIR})
if(MACOSX_ODBC_DEPRECATED)
  add_definitions(-Wno-deprecated-declarations)
endif()

ROOT_GENERATE_DICTIONARY(G__ODBC *.h MODULE RODBC LINKDEF LinkDef.h)

ROOT_LINKER_LIBRARY(RODBC *.cxx G__ODBC.cxx LIBRARIES Core ${ODBC_LIBRARIES} DEPENDENCIES Net RIO)
ROOT_INSTALL_HEADERS()
