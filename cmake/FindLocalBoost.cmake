find_path(
  Boost_INCLUDE_DIRS
  NAMES assert.hpp
  PATH_SUFFIXES "include/boost"
)

find_library(
    Boost_LIBRARIES
    NAMES boost
    PATH_SUFFIXES lib lib64
)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost REQUIRED_VARS Boost_LIBRARIES Boost_INCLUDE_DIRS)