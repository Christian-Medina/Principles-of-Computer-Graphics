///////////////////////////////////////////////////////////////////////////////
// gfxppm.hh
//
// Read/write Portable PixMap (PPM) files to/from our image class.
//
// Documentation on the PPM format:
// https://en.wikipedia.org/wiki/Netpbm_format
// http://netpbm.sourceforge.net/doc/ppm.html
//
// This module builds on gfximage.hh, so familiarize yourself with
// that file before using this one.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>

#include "gfxcolor.hh"
#include "gfximage.hh"

namespace gfx {

  // Write image to a PPM file at path. When binary_samples is true,
  // use binary samples (aka "raw" or "P6" mode). This is more
  // space-efficient so is the default behavior. When binary_samples
  // is false, use human-readable text samples (aka "ASCII" or "P3"
  // mode). Return true on success, or false on I/O error.
  bool ppm_write(const true_color_image& image,
		 const std::string& path,
		 bool binary_samples = true) {

    // Writing is more straightforward since we don't need to deal
    // with comments or format checking, so we present it first.

    // Open the file for writing, or fail.
    std::ofstream f;
    if (binary_samples) {
      f.open(path, std::ios_base::binary);
    } else {
      f.open(path);
    }
    if (!f) {
      return false;
    }

    // Decide which magic string to use.
    const std::string& magic = binary_samples ? "P6" : "P3";

    // Image header. We hardcode maxval to 255.
    f << magic
      << ' '
      << image.width()
      << ' '
      << image.height()
      << ' '
      << 255
      << '\n';

    // Write pixels in top-to-bottom order.
    for (int y = 0; y < image.height(); ++y) {
      for (int x = 0; x < image.width(); ++x) {

	const gfx::true_color_rgb& pixel = image.pixel(x, y);

	// Decide how to write this rgb.
	if (binary_samples) {
	  // Binary, so three unsigned bytes.
	  uint8_t bytes[3];
	  for (int i = 0; i < 3; ++i) {
	    bytes[i] = pixel[i];
	  }
	  // Note that we are forced into a rather barbaric typecast
	  // here, in order to ensure that f.write(...) does not
	  // despoil the most significant bit of our bytes.
	  f.write((char*) bytes, 3);
	} else {
          // Text, so write decimal representations of the three
          // components, separated by spaces. Note that this adds a
          // space before the first pixel, which is permissible
          // according to the PPM standard. Further note that we need
          // to cast the uint8_t intensity values to int so that they
          // are printed as decimal numbers and not ASCII codes.
          f << ' ' << int(pixel.red())
	    << ' ' << int(pixel.green())
	    << ' ' << int(pixel.blue());
	}
      }
      // The standard specifies that no line should be longer than 70
      // characters. We play it safe and write only one pixel per
      // line.
      if (!binary_samples) {
	f << '\n';
      }
    }

    // Close the file or fail.
    if (!f) {
      f.close();
      return false;
    }

    // Success.
    f.close();
    return true;
  }

  // Read a PPM file at path. This function can decode both
  // binary/raw/P6 and textual/ASCII/P3 PPM variants. On success, fill
  // result with the contents of the image file and return true. On
  // failure, make result empty and return false. Failure conditions
  // include file-not-found, I/O error, and a file that is not in
  // proper PPM format.
  bool ppm_read(true_color_image& result,
		const std::string& path) {

    // Open the file or fail.
    std::ifstream f(path, std::ios_base::binary);
    if (!f) {
      result.clear();
      return false;
    }

    // Helper function to skip whitespace characters in f.
    auto skip_whitespace = [&]() {
      bool matched = false;
      while (isspace(f.peek())) {
	f.get();
	matched = true;
      }
      return matched;
    };

    // Helper function to skip a comment in f.
    auto skip_comment = [&]() {
      bool matched = false;
      while ((f.peek() == '#')) {
	std::string comment;
	std::getline(f, comment);
	matched = true;
      }
      return matched;
    };

    // Helper function to skip any combination of whitespace and
    // comments.
    auto skip_whitespace_and_comments = [&]() {
      while (skip_whitespace() || skip_comment())
	;
    };

    // Read the first two characters, which should contain a magic
    // string "P3" or "P6".
    std::string magic;
    {
      // Temporarily read into a C-style string.
      char magic_cstr[3];
      f.read(magic_cstr, 2);
      if (!f) {
	result.clear();
	f.close();
	return false;
      }
      magic.assign(magic_cstr, 2);
    }

    // Decide whether this is a binary PPM, textual PPM, or neither.
    bool binary_samples;
    if (magic == "P6") {
      binary_samples = true;
    } else if (magic == "P3") {
      binary_samples = false;
    } else {
      // Fail.
      result.clear();
      f.close();
      return false;
    }

    // Leading whitespace.
    skip_whitespace_and_comments();

    // Width.
    int width;
    f >> width;
    skip_whitespace_and_comments();

    // Height.
    int height;
    f >> height;
    skip_whitespace_and_comments();

    // Maxval.
    int maxval;
    f >> maxval;

    // The specification says that, after maxval, there is exactly one
    // whitespace character, so we read it. In particular we do not
    // use skip_whitespace_and_comments() here.
    char single_whitespace = f.get();

    // Check that all those I/O operations succeeded and that all
    // header values are valid.
    if (!f ||
	(width <= 0) ||
	(height <= 0) ||
	(maxval <= 0) ||
	(maxval >= 65536) ||
	(!isspace(single_whitespace))) {
      // Nope, fail.
      f.close();
      return false;
    }

    // Now that we know we have legitimate width and height, resize
    // result.
    result.resize(width, height);

    // Read pixels in top-to-bottom order.
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
	// Read the three RGB components.
	for (int s = 0; s < 3; ++s) {
	  // We will initialize raw_sample to an intensity value
	  // [0, 255], either by reading a binary sample or textual
	  // sample.
	  int raw_sample;

	  // Decide how to read the sample.
	  if (binary_samples) {
	    if (maxval < 256) {
	      // One-byte sample. As with ppm_write, we need to resort
	      // to a vulgar typecast to preserve the sign bit.
	      uint8_t byte[1];
	      f.read((char*) byte, 1);
	      raw_sample = byte[0];
	      assert((raw_sample >= 0) && (raw_sample <= 255));
	    } else {
              // Two-byte sample, including another typecast.
	      uint8_t bytes[2];
	      f.read((char*) bytes, 2);
	      // The most-significant byte comes first. Note that we
	      // cast to int before shifting, because otherwise we
	      // would shift a uint8_t left 8 times which always
	      // yields 0.
	      raw_sample = (int(bytes[0]) << 8) | int(bytes[1]);
	      assert((raw_sample >= 0) && (raw_sample <= 0xFFFF));
	    }
	  } else {
	    // text samples
	    f >> raw_sample;
	  }

	  // Normalize raw_sample from [0, maxval] to [0, 255]. Note
	  // that we multiply before dividing since raw_sample/maval
	  // is always either 0 or 1 due to integer division
	  // truncation. Also, note that we are using int instead of
	  // uint8_t so that the (raw_sample * 255) expression does
	  // not overflow.
	  int normalized_sample = (raw_sample * 255) / maxval;
	  assert(normalized_sample >= 0);
	  assert(normalized_sample <= 255);

	  // Finally copy this intensity value into the image object.
	  result.pixel(x, y)[s] = normalized_sample;
	}
      }
    }

    // If any of those I/O operations failed, the whole process fails.
    if (!f) {
      result.clear();
      f.close();
      return false;
    }

    // Success.
    f.close();
    return true;
  }
}
