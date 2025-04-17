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

# This library only exists to make sure the sample code actually compiles

file(GLOB SAMPLE_CODE_APP_SRCS *.cpp *.h *.hpp)
add_library(sample_code_app ${SAMPLE_CODE_APP_SRCS})
target_link_libraries(sample_code_app
  PUBLIC
    vrslib
    vrs_logging
    vrs_os
  PRIVATE
    ocean
    ffmpeg_decoding
)

# Specify the installation path for FFmpeg
set(FFmpeg_ROOT "$ENV{HOME}/vrs_third_party_libs/ffmpeg")

# Set the include directories
set(FFmpeg_INCLUDE_DIRS
    "${FFmpeg_ROOT}/include"
)

#add_library(ffmpeg_avutil_2 STATIC IMPORTED)
#set_target_properties(ffmpeg_avutil_2 PROPERTIES
#    IMPORTED_LOCATION "${FFmpeg_ROOT}/lib/libavutil.a"
#)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

# Add an executable for OceanTest
add_executable(OceanTest OceanTest.cpp)
target_link_libraries(OceanTest PRIVATE ocean)

# Add an executable for FFmpegTest
add_executable(FFmpegTest FFmpegTest.cpp)
target_link_libraries(FFmpegTest PRIVATE ffmpeg_decoding)
# target_include_directories(FFmpegTest PRIVATE ${AVUTIL_INCLUDE_DIR})
