// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Demux API.
// Enables extraction of image and extended format data from WebP files.

// Code Example: Demuxing WebP data to extract all the frames, ICC profile
// and EXIF/XMP metadata.
/*
  WebPDemuxer* demux = WebPDemux(&webp_data);

  uint32_t width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
  uint32_t height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);
  // ... (Get information about the features present in the WebP file).
  uint32_t flags = WebPDemuxGetI(demux, WEBP_FF_FORMAT_FLAGS);

  // ... (Iterate over all frames).
  WebPIterator iter;
  if (WebPDemuxGetFrame(demux, 1, &iter)) {
    do {
      // ... (Consume 'iter'; e.g. Decode 'iter.fragment' with WebPDecode(),
      // ... and get other frame properties like width, height, offsets etc.
      // ... see 'struct WebPIterator' below for more info).
    } while (WebPDemuxNextFrame(&iter));
    WebPDemuxReleaseIterator(&iter);
  }

  // ... (Extract metadata).
  WebPChunkIterator chunk_iter;
  if (flags & ICCP_FLAG) WebPDemuxGetChunk(demux, "ICCP", 1, &chunk_iter);
  // ... (Consume the ICC profile in 'chunk_iter.chunk').
  WebPDemuxReleaseChunkIterator(&chunk_iter);
  if (flags & EXIF_FLAG) WebPDemuxGetChunk(demux, "EXIF", 1, &chunk_iter);
  // ... (Consume the EXIF metadata in 'chunk_iter.chunk').
  WebPDemuxReleaseChunkIterator(&chunk_iter);
  if (flags & XMP_FLAG) WebPDemuxGetChunk(demux, "XMP ", 1, &chunk_iter);
  // ... (Consume the XMP metadata in 'chunk_iter.chunk').
  WebPDemuxReleaseChunkIterator(&chunk_iter);
  WebPDemuxDelete(demux);
*/

#ifndef MV_WEBP_WEBP_DEMUX_H_
#define MV_WEBP_WEBP_DEMUX_H_

#include "./decode.h"     // for MV_WEBP_CSP_MODE
#include "./mux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MV_WEBP_DEMUX_ABI_VERSION 0x0107    // MAJOR(8b) + MINOR(8b)

// Note: forward declaring enumerations is not allowed in (strict) C and C++,
// the types are left here for reference.
// typedef enum WebPDemuxState WebPDemuxState;
// typedef enum WebPFormatFeature WebPFormatFeature;
typedef struct MV_WebPDemuxer WebPDemuxer;
typedef struct MV_WebPIterator WebPIterator;
typedef struct MV_WebPChunkIterator WebPChunkIterator;
typedef struct MV_WebPAnimInfo WebPAnimInfo;
typedef struct MV_WebPAnimDecoderOptions WebPAnimDecoderOptions;

//------------------------------------------------------------------------------

// Returns the version number of the demux library, packed in hexadecimal using
// 8bits for each of major/minor/revision. E.g: v2.5.7 is 0x020507.
MV_WEBP_EXTERN(int) MV_WebPGetDemuxVersion(void);

//------------------------------------------------------------------------------
// Life of a Demux object

typedef enum MV_WebPDemuxState {
  MV_WEBP_DEMUX_PARSE_ERROR    = -1,  // An error occurred while parsing.
  MV_WEBP_DEMUX_PARSING_HEADER =  0,  // Not enough data to parse full header.
  MV_WEBP_DEMUX_PARSED_HEADER  =  1,  // Header parsing complete,
                                   // data may be available.
  MV_WEBP_DEMUX_DONE           =  2   // Entire file has been parsed.
} MV_WebPDemuxState;

// Internal, version-checked, entry point
MV_WEBP_EXTERN(MV_WebPDemuxer*) MV_WebPDemuxInternal(
    const WebPData*, int, MV_WebPDemuxState*, int);

// Parses the full WebP file given by 'data'. For single images the WebP file
// header alone or the file header and the chunk header may be absent.
// Returns a WebPDemuxer object on successful parse, NULL otherwise.
static MV_WEBP_INLINE MV_WebPDemuxer* MV_WebPDemux(const MV_WebPData* data) {
  return MV_WebPDemuxInternal(data, 0, NULL, WEBP_DEMUX_ABI_VERSION);
}

// Parses the possibly incomplete WebP file given by 'data'.
// If 'state' is non-NULL it will be set to indicate the status of the demuxer.
// Returns NULL in case of error or if there isn't enough data to start parsing;
// and a WebPDemuxer object on successful parse.
// Note that WebPDemuxer keeps internal pointers to 'data' memory segment.
// If this data is volatile, the demuxer object should be deleted (by calling
// WebPDemuxDelete()) and WebPDemuxPartial() called again on the new data.
// This is usually an inexpensive operation.
static MV_WEBP_INLINE MV_WebPDemuxer* MV_WebPDemuxPartial(
    const MV_WebPData* data, MV_WebPDemuxState* state) {
  return MV_WebPDemuxInternal(data, 1, state, MV_WEBP_DEMUX_ABI_VERSION);
}

// Frees memory associated with 'dmux'.
MV_WEBP_EXTERN(void) MV_WebPDemuxDelete(MV_WebPDemuxer* dmux);

//------------------------------------------------------------------------------
// Data/information extraction.

typedef enum MV_WebPFormatFeature {
  MV_WEBP_FF_FORMAT_FLAGS,  // Extended format flags present in the 'VP8X' chunk.
  MV_WEBP_FF_CANVAS_WIDTH,
  MV_WEBP_FF_CANVAS_HEIGHT,
  MV_WEBP_FF_LOOP_COUNT,
  MV_WEBP_FF_BACKGROUND_COLOR,
  MV_WEBP_FF_FRAME_COUNT    // Number of frames present in the demux object.
                         // In case of a partial demux, this is the number of
                         // frames seen so far, with the last frame possibly
                         // being partial.
} MV_WebPFormatFeature;

// Get the 'feature' value from the 'dmux'.
// NOTE: values are only valid if WebPDemux() was used or WebPDemuxPartial()
// returned a state > WEBP_DEMUX_PARSING_HEADER.
MV_WEBP_EXTERN(uint32_t) MV_WebPDemuxGetI(
    const MV_WebPDemuxer* dmux, MV_WebPFormatFeature feature);

//------------------------------------------------------------------------------
// Frame iteration.

struct MV_WebPIterator {
  int frame_num;
  int num_frames;          // equivalent to WEBP_FF_FRAME_COUNT.
  int x_offset, y_offset;  // offset relative to the canvas.
  int width, height;       // dimensions of this frame.
  int duration;            // display duration in milliseconds.
  MV_WebPMuxAnimDispose dispose_method;  // dispose method for the frame.
  int complete;   // true if 'fragment' contains a full frame. partial images
                  // may still be decoded with the WebP incremental decoder.
  MV_WebPData fragment;  // The frame given by 'frame_num'. Note for historical
                      // reasons this is called a fragment.
  int has_alpha;      // True if the frame contains transparency.
  MV_WebPMuxAnimBlend blend_method;  // Blend operation for the frame.

  uint32_t pad[2];         // padding for later use.
  void* private_;          // for internal use only.
};

// Retrieves frame 'frame_number' from 'dmux'.
// 'iter->fragment' points to the frame on return from this function.
// Setting 'frame_number' equal to 0 will return the last frame of the image.
// Returns false if 'dmux' is NULL or frame 'frame_number' is not present.
// Call WebPDemuxReleaseIterator() when use of the iterator is complete.
// NOTE: 'dmux' must persist for the lifetime of 'iter'.
MV_WEBP_EXTERN(int) MV_WebPDemuxGetFrame(
    const MV_WebPDemuxer* dmux, int frame_number, MV_WebPIterator* iter);

// Sets 'iter->fragment' to point to the next ('iter->frame_num' + 1) or
// previous ('iter->frame_num' - 1) frame. These functions do not loop.
// Returns true on success, false otherwise.
MV_WEBP_EXTERN(int) MV_WebPDemuxNextFrame(MV_WebPIterator* iter);
MV_WEBP_EXTERN(int) MV_WebPDemuxPrevFrame(MV_WebPIterator* iter);

// Releases any memory associated with 'iter'.
// Must be called before any subsequent calls to WebPDemuxGetChunk() on the same
// iter. Also, must be called before destroying the associated WebPDemuxer with
// WebPDemuxDelete().
MV_WEBP_EXTERN(void) MV_WebPDemuxReleaseIterator(MV_WebPIterator* iter);

//------------------------------------------------------------------------------
// Chunk iteration.

struct MV_WebPChunkIterator {
  // The current and total number of chunks with the fourcc given to
  // WebPDemuxGetChunk().
  int chunk_num;
  int num_chunks;
  MV_WebPData chunk;    // The payload of the chunk.

  uint32_t pad[6];   // padding for later use
  void* private_;
};

// Retrieves the 'chunk_number' instance of the chunk with id 'fourcc' from
// 'dmux'.
// 'fourcc' is a character array containing the fourcc of the chunk to return,
// e.g., "ICCP", "XMP ", "EXIF", etc.
// Setting 'chunk_number' equal to 0 will return the last chunk in a set.
// Returns true if the chunk is found, false otherwise. Image related chunk
// payloads are accessed through WebPDemuxGetFrame() and related functions.
// Call WebPDemuxReleaseChunkIterator() when use of the iterator is complete.
// NOTE: 'dmux' must persist for the lifetime of the iterator.
MV_WEBP_EXTERN(int) MV_WebPDemuxGetChunk(const MV_WebPDemuxer* dmux,
                                   const char fourcc[4], int chunk_number,
                                   MV_WebPChunkIterator* iter);

// Sets 'iter->chunk' to point to the next ('iter->chunk_num' + 1) or previous
// ('iter->chunk_num' - 1) chunk. These functions do not loop.
// Returns true on success, false otherwise.
MV_WEBP_EXTERN(int) MV_WebPDemuxNextChunk(MV_WebPChunkIterator* iter);
MV_WEBP_EXTERN(int) MV_WebPDemuxPrevChunk(MV_WebPChunkIterator* iter);

// Releases any memory associated with 'iter'.
// Must be called before destroying the associated WebPDemuxer with
// WebPDemuxDelete().
MV_WEBP_EXTERN(void) MV_WebPDemuxReleaseChunkIterator(MV_WebPChunkIterator* iter);

//------------------------------------------------------------------------------
// WebPAnimDecoder API
//
// This API allows decoding (possibly) animated WebP images.
//
// Code Example:
/*
  WebPAnimDecoderOptions dec_options;
  WebPAnimDecoderOptionsInit(&dec_options);
  // Tune 'dec_options' as needed.
  WebPAnimDecoder* dec = WebPAnimDecoderNew(webp_data, &dec_options);
  WebPAnimInfo anim_info;
  WebPAnimDecoderGetInfo(dec, &anim_info);
  for (uint32_t i = 0; i < anim_info.loop_count; ++i) {
    while (WebPAnimDecoderHasMoreFrames(dec)) {
      uint8_t* buf;
      int timestamp;
      WebPAnimDecoderGetNext(dec, &buf, &timestamp);
      // ... (Render 'buf' based on 'timestamp').
      // ... (Do NOT free 'buf', as it is owned by 'dec').
    }
    WebPAnimDecoderReset(dec);
  }
  const WebPDemuxer* demuxer = WebPAnimDecoderGetDemuxer(dec);
  // ... (Do something using 'demuxer'; e.g. get EXIF/XMP/ICC data).
  WebPAnimDecoderDelete(dec);
*/

typedef struct MV_WebPAnimDecoder MV_WebPAnimDecoder;  // Main opaque object.

// Global options.
struct MV_WebPAnimDecoderOptions {
  // Output colorspace. Only the following modes are supported:
  // MODE_RGBA, MODE_BGRA, MODE_rgbA and MODE_bgrA.
  MV_WEBP_CSP_MODE color_mode;
  int use_threads;           // If true, use multi-threaded decoding.
  uint32_t padding[7];       // Padding for later use.
};

// Internal, version-checked, entry point.
MV_WEBP_EXTERN(int) MV_WebPAnimDecoderOptionsInitInternal(
    MV_WebPAnimDecoderOptions*, int);

// Should always be called, to initialize a fresh WebPAnimDecoderOptions
// structure before modification. Returns false in case of version mismatch.
// WebPAnimDecoderOptionsInit() must have succeeded before using the
// 'dec_options' object.
static MV_WEBP_INLINE int MV_WebPAnimDecoderOptionsInit(
    MV_WebPAnimDecoderOptions* dec_options) {
  return MV_WebPAnimDecoderOptionsInitInternal(dec_options,
                                            MV_WEBP_DEMUX_ABI_VERSION);
}

// Internal, version-checked, entry point.
MV_WEBP_EXTERN(MV_WebPAnimDecoder*) MV_WebPAnimDecoderNewInternal(
    const MV_WebPData*, const MV_WebPAnimDecoderOptions*, int);

// Creates and initializes a WebPAnimDecoder object.
// Parameters:
//   webp_data - (in) WebP bitstream. This should remain unchanged during the
//                    lifetime of the output WebPAnimDecoder object.
//   dec_options - (in) decoding options. Can be passed NULL to choose
//                      reasonable defaults (in particular, color mode MODE_RGBA
//                      will be picked).
// Returns:
//   A pointer to the newly created WebPAnimDecoder object, or NULL in case of
//   parsing error, invalid option or memory error.
static MV_WEBP_INLINE MV_WebPAnimDecoder* MV)WebPAnimDecoderNew(
    const MV_WebPData* webp_data, const MV_WebPAnimDecoderOptions* dec_options) {
  return MV_WebPAnimDecoderNewInternal(webp_data, dec_options,
                                    MV_WEBP_DEMUX_ABI_VERSION);
}

// Global information about the animation..
struct MV_WebPAnimInfo {
  uint32_t canvas_width;
  uint32_t canvas_height;
  uint32_t loop_count;
  uint32_t bgcolor;
  uint32_t frame_count;
  uint32_t pad[4];   // padding for later use
};

// Get global information about the animation.
// Parameters:
//   dec - (in) decoder instance to get information from.
//   info - (out) global information fetched from the animation.
// Returns:
//   True on success.
MV_WEBP_EXTERN(int) MV_WebPAnimDecoderGetInfo(const MV_WebPAnimDecoder* dec,
                                        MV_WebPAnimInfo* info);

// Fetch the next frame from 'dec' based on options supplied to
// WebPAnimDecoderNew(). This will be a fully reconstructed canvas of size
// 'canvas_width * 4 * canvas_height', and not just the frame sub-rectangle. The
// returned buffer 'buf' is valid only until the next call to
// WebPAnimDecoderGetNext(), WebPAnimDecoderReset() or WebPAnimDecoderDelete().
// Parameters:
//   dec - (in/out) decoder instance from which the next frame is to be fetched.
//   buf - (out) decoded frame.
//   timestamp - (out) timestamp of the frame in milliseconds.
// Returns:
//   False if any of the arguments are NULL, or if there is a parsing or
//   decoding error, or if there are no more frames. Otherwise, returns true.
MV_WEBP_EXTERN(int) MV_WebPAnimDecoderGetNext(MV_WebPAnimDecoder* dec,
                                        uint8_t** buf, int* timestamp);

// Check if there are more frames left to decode.
// Parameters:
//   dec - (in) decoder instance to be checked.
// Returns:
//   True if 'dec' is not NULL and some frames are yet to be decoded.
//   Otherwise, returns false.
MV_WEBP_EXTERN(int) MV_WebPAnimDecoderHasMoreFrames(const MV_WebPAnimDecoder* dec);

// Resets the WebPAnimDecoder object, so that next call to
// WebPAnimDecoderGetNext() will restart decoding from 1st frame. This would be
// helpful when all frames need to be decoded multiple times (e.g.
// info.loop_count times) without destroying and recreating the 'dec' object.
// Parameters:
//   dec - (in/out) decoder instance to be reset
MV_WEBP_EXTERN(void) MV_WebPAnimDecoderReset(MV_WebPAnimDecoder* dec);

// Grab the internal demuxer object.
// Getting the demuxer object can be useful if one wants to use operations only
// available through demuxer; e.g. to get XMP/EXIF/ICC metadata. The returned
// demuxer object is owned by 'dec' and is valid only until the next call to
// WebPAnimDecoderDelete().
//
// Parameters:
//   dec - (in) decoder instance from which the demuxer object is to be fetched.
MV_WEBP_EXTERN(const MV_WebPDemuxer*) MV_WebPAnimDecoderGetDemuxer(
    const MV_WebPAnimDecoder* dec);

// Deletes the WebPAnimDecoder object.
// Parameters:
//   dec - (in/out) decoder instance to be deleted
MV_WEBP_EXTERN(void) MV_WebPAnimDecoderDelete(MV_WebPAnimDecoder* dec);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_WEBP_DEMUX_H_ */
