// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <vrs/utils/DecoderFactory.h>
#include <vrs/utils/xprs_decoder/XprsDecoder.h>

namespace vrstool {

inline void SharedInit() {
  ::vrs::utils::DecoderFactory::get().registerDecoderMaker(::vrs::vxprs::xprsDecoderMaker);
}

} // namespace vrstool
