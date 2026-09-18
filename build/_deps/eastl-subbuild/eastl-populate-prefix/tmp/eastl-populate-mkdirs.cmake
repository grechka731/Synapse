# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/grch/code/engine/build/_deps/eastl-src")
  file(MAKE_DIRECTORY "/home/grch/code/engine/build/_deps/eastl-src")
endif()
file(MAKE_DIRECTORY
  "/home/grch/code/engine/build/_deps/eastl-build"
  "/home/grch/code/engine/build/_deps/eastl-subbuild/eastl-populate-prefix"
  "/home/grch/code/engine/build/_deps/eastl-subbuild/eastl-populate-prefix/tmp"
  "/home/grch/code/engine/build/_deps/eastl-subbuild/eastl-populate-prefix/src/eastl-populate-stamp"
  "/home/grch/code/engine/build/_deps/eastl-subbuild/eastl-populate-prefix/src"
  "/home/grch/code/engine/build/_deps/eastl-subbuild/eastl-populate-prefix/src/eastl-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/grch/code/engine/build/_deps/eastl-subbuild/eastl-populate-prefix/src/eastl-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/grch/code/engine/build/_deps/eastl-subbuild/eastl-populate-prefix/src/eastl-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
