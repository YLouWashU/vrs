# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# - Find Ocean
# Find the Ocean library
#
# Ocean_INCLUDE_DIRS - where to find ocean.h, etc.
# Ocean_LIBRARIES - List of libraries when using Ocean.
# Ocean_FOUND - True if Ocean found.

# Specify the installation path for Ocean
set(Ocean_ROOT "$ENV{HOME}/vrs_third_party_libs/ocean")

# Detect the platform and set the appropriate subdirectory
if(APPLE)
    set(Ocean_PLATFORM "macos_static_Release")
elseif(UNIX)
    set(Ocean_PLATFORM "linux_static_Release")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Set the include directories
set(Ocean_INCLUDE_DIRS
    "${Ocean_ROOT}/${Ocean_PLATFORM}/include"
#    "${Ocean_ROOT}/${Ocean_PLATFORM}/include/ocean/base"
#    "${Ocean_ROOT}/${Ocean_PLATFORM}/include/ocean/cv"
#    "${Ocean_ROOT}/${Ocean_PLATFORM}/include/ocean/math"
)

# Create imported targets for each static library
add_library(ocean_base STATIC IMPORTED)
set_target_properties(ocean_base PROPERTIES
    IMPORTED_LOCATION "${Ocean_ROOT}/${Ocean_PLATFORM}/lib/libocean_base.a"
)
add_library(ocean_cv STATIC IMPORTED)
set_target_properties(ocean_cv PROPERTIES
    IMPORTED_LOCATION "${Ocean_ROOT}/${Ocean_PLATFORM}/lib/libocean_cv.a"
)
add_library(ocean_math STATIC IMPORTED)
set_target_properties(ocean_math PROPERTIES
    IMPORTED_LOCATION "${Ocean_ROOT}/${Ocean_PLATFORM}/lib/libocean_math.a"
)

# Create an imported target for the Ocean library
add_library(ocean INTERFACE)
target_link_libraries(ocean INTERFACE
    ocean_base
    ocean_cv
    ocean_math
)
# Set the include directories for the interface target
target_include_directories(ocean INTERFACE "${Ocean_INCLUDE_DIRS}")

message(STATUS "Found Ocean lib, with include dirs: ${Ocean_INCLUDE_DIRS}")
