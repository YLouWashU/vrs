/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <limits>
#include <map>
#include <tuple>

#include "ContentBlockReader.h"
#include "DataLayout.h"
#include "RecordFormat.h"
#include "StreamPlayer.h"

namespace vrs {

using std::map;
using std::pair;
using std::string;
using std::tuple;
using std::unique_ptr;
using std::vector;

class DataLayoutBlockReader;
class ImageBlockReader;
class AudioBlockReader;

namespace datalayout_conventions {
class VideoFrameSpec;
}

/// \brief Internal data structure to hold various objects needed to decode a specific RecordFormat.
/// @internal
struct RecordFormatReader {
  double lastReadRecordTimestamp = std::numeric_limits<double>::max();
  RecordFormat recordFormat;
  vector<unique_ptr<ContentBlockReader>> contentReaders;
  vector<unique_ptr<DataLayout>> expectedDataLayouts;
  vector<unique_ptr<DataLayout>> legacyDataLayouts;
};

/// \brief Specialized StreamPlayer designed to handle records which format is managed by
/// RecordFormat, which are a succession of typed content blocks described by ContentBlock objects.
///
/// For each of the record's content blocks a specific callback will be called depending on the type
/// of content block. The content blocks are decoded in order, until they are all read, some
/// decoding error occurred, or a callback returns false.
/// The block's index is passed in case you need to disambiguate successive blocks of the same type,
/// or need to know when a new block is started.
class RecordFormatStreamPlayer : public StreamPlayer {
 public:
  /// Callback for DataLayout content blocks, after it was read.
  /// @param record: Metadata associated with the record being read.
  /// @param blockIndex: Index of the content block being read.
  /// @param dl: DataLayout read.
  /// @return Return true if remaining record content blocks should be read.
  /// Return false, if the record should not be read entirely, for instance, if you only need to
  /// read some metadata stored in the first content block, but don't need to read & decode the rest
  /// of the record.
  virtual bool onDataLayoutRead(const CurrentRecord& record, size_t /* blockIndex */, DataLayout&) {
    return true; // we can go read the next block, if any, since we've read the data
  }

  /// Callback for image content blocks.
  /// The ContentBlock object 'cb' describes the image content block, but the content block's data
  /// has not been read yet. Query the ContentBlock object to know the details about the image and
  /// the content block's size. Then you can allocate or reuse a buffer, and call
  /// record.reader->read() to read the content block's data in that buffer.
  /// Depending on cb.image().getImageFormat(), the content block's data will be:
  /// ImageFormat::RAW: raw pixel buffer data, in the format specified by cb.image.getPixelFormat().
  /// ImageFormat::JPG: JPG data, as it found in a regular .jpg file.
  /// ImageFormat::PNG: PNG data, as it found in a regular .png file.
  /// ImageFormat::VIDEO: compressed data, as generated by the video codec cb.image.getCodecName().
  /// Preferably, use a VideoRecordFormatStreamPlayer object to decode such records.
  /// @param record: Metadata associated with the record being read.
  /// @param blockIndex: Index of the content block being read.
  /// @param cb: ContentBlock describing the image data to be read.
  /// @return Return true if remaining record content blocks should be read.
  virtual bool onImageRead(const CurrentRecord& record, size_t blockIndex, const ContentBlock& cb) {
    return onUnsupportedBlock(record, blockIndex, cb);
  }

  /// Callback for audio content blocks.
  /// The ContentBlock object 'cb' describes the audio content block, but the content block's data
  /// has not been read yet. Query the ContentBlock object to know the details about the audio data
  /// and the content block's size. Then you can allocate or reuse a buffer, and call
  /// record.reader->read() to read the content block's data in that buffer.
  /// @param record: Metadata associated with the record being read.
  /// @param blockIndex: Index of the content block being read.
  /// @param cb: ContentBlock describing the audio data to be read.
  /// @return Return true if remaining record content blocks should be read.
  virtual bool onAudioRead(const CurrentRecord& record, size_t blockIndex, const ContentBlock& cb) {
    return onUnsupportedBlock(record, blockIndex, cb);
  }

  /// Callback for custom content blocks.
  /// The ContentBlock object 'cb' describes the custom content block, but the content block's data
  /// has not been read yet. Query the ContentBlock object to know the details about the custom data
  /// and the content block's size. Then you can allocate or reuse a buffer, and call
  /// record.reader->read() to read the content block's data in that buffer.
  /// @param rec: Metadata associated with the record being read.
  /// @param blkIdx: Index of the content block being read.
  /// @param cb: ContentBlock describing the data to be read.
  /// @return Return true if remaining record content blocks should be read.
  virtual bool onCustomBlockRead(const CurrentRecord& rec, size_t blkIdx, const ContentBlock& cb) {
    return onUnsupportedBlock(rec, blkIdx, cb);
  }

  /// Callback for unsupported or unrecognized content blocks.
  /// If the size of a content block can't be determined, and the content block can not be decoded
  /// safely, this callback is called instead of any of the preceding callbacks.
  /// This callback is also called when the other callbacks aren't implemented.
  /// @param rec: Metadata associated with the record being read.
  /// @param blkIdx: Index of the block being read.
  /// @param cb: ContentBlock describing the data to be read.
  /// @return Return true if remaining record content blocks should be read.
  virtual bool onUnsupportedBlock(const CurrentRecord& rec, size_t blkIdx, const ContentBlock& cb);

  /// Callback called when the object is attached to a RecordFileReader object, so that this object
  /// can initialize itself. For RecordFormatStreamPlayer internal use.
  /// Do not prevent this initialization, or the record won't be read correctly.
  /// @param recordFileReader: Record file reader this stream player was just attached to.
  /// @param streamId: StreamId of the stream of records this stream player is associated with.
  /// @internal
  void onAttachedToFileReader(RecordFileReader& recordFileReader, StreamId streamId) override;

  /// Callback called when a record is read. For RecordFormatStreamPlayer internal use.
  /// @internal
  bool processRecordHeader(const CurrentRecord& record, DataReference& outDataReference) override;
  /// Callback called when a record is read. For RecordFormatStreamPlayer internal use.
  /// @internal
  void processRecord(const CurrentRecord& record, uint32_t readSize) override;

  /// For RecordFormatStreamPlayer internal use.
  /// @internal
  RecordFormatReader* getLastRecordFormatReader(StreamId id, Record::Type recordType) const;

  /// For RecordFormatStreamPlayer internal use.
  /// @internal
  RecordFormatReader* getCurrentRecordFormatReader() const {
    return currentReader_;
  }

 protected:
  // Helper class, to be used exclusively during onXXXRead() callbacks,
  // to get the wished for DataLayout
  template <class T>
  inline T& getExpectedLayout(DataLayout& layout, size_t blockIndex) {
    return getCachedLayout<T>(currentReader_->expectedDataLayouts, layout, blockIndex);
  }
  // Helper class, to be used exclusively during onXXXRead() callbacks,
  // to get legacy fields no longer present in the official layout, for backward compatibility needs
  template <class T>
  inline T& getLegacyLayout(DataLayout& layout, size_t blockIndex) {
    return getCachedLayout<T>(currentReader_->legacyDataLayouts, layout, blockIndex);
  }
  template <class T>
  T& getCachedLayout(
      vector<unique_ptr<DataLayout>>& layoutCache,
      DataLayout& layout,
      size_t blockIndex) {
    if (layoutCache.size() <= blockIndex) {
      layoutCache.resize(blockIndex + 1);
    }
    if (!layoutCache[blockIndex]) {
      T* expectedLayout = new T;
      layoutCache[blockIndex].reset(expectedLayout);
      expectedLayout->mapLayout(layout);
    }
    return reinterpret_cast<T&>(*layoutCache[blockIndex].get());
  }

  RecordFileReader* recordFileReader_{};

  // Keep the readers all separate,
  // in case the same RecordFormatStreamPlayer is handling multiple streams.
  map<tuple<StreamId, Record::Type, uint32_t>, RecordFormatReader> readers_;
  map<pair<StreamId, Record::Type>, RecordFormatReader*> lastReader_;
  RecordFormatReader* currentReader_{};
};

} // namespace vrs
