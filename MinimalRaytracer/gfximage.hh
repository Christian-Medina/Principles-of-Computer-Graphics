///////////////////////////////////////////////////////////////////////////////
// gfximage.hh
//
// Data structure for a raster image. The image<color_depth_type>
// template class stores a raster image encoded in the specified color
// depth.
//
// This module builds on gfxcolor.hh, so familiarize yourself with
// that file before using this one.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include "gfxcolor.hh"

namespace gfx {

  // A raster image, with a width, height, and (width x height)
  // pixels. Each pixel is an rgb<color_depth>. Ordinarily an image is
  // non-empty, with positive width, positive height, and a non-zero
  // number of pixels. However an image object may also be empty with
  // zero width, zero height, and zero pixels. The empty state exists
  // primarily so that the default constructor can have well-defined
  // semantics.
  template <typename color_depth_parameter>
  class image {
  public:

    // Type aliases.
    using color_depth = color_depth_parameter;
    using same_type = image<color_depth>;
    using rgb_type = rgb<color_depth>;
    using row_type = std::vector<rgb_type>;

    // Default constructor. Creates an empty image.
    image() {
      assert(empty());
    }

    // Construct an image with the given width and height, both of
    // which must be positive. Every pixel is initialized to
    // default_color.
    image(int width,
	  int height,
	  const rgb_type& default_color = BLACK.convert_to<color_depth>()) {
      
      assert(width > 0);
      assert(height > 0);
      
      _rows.assign(height, row_type(width, default_color));
    }

    // Copy constructor.
    image(const same_type& rhs)
      : _rows(rhs._rows) { }

    // Assignment operator.
    same_type& operator=(const same_type& rhs) {
      _rows = rhs._rows;
      return *this;
    }      

    // Equality operator.
    bool operator==(const same_type& rhs) const {
      return ((empty() == rhs.empty()) &&
	      std::equal(_rows.begin(), _rows.end(), rhs._rows.begin()));
    }

    // Non-equality operator.
    bool operator!=(const same_type& rhs) const {
      return ! (*this == rhs);
    }

    // Determine whether this image is almost equal to rhs. Returns
    // true when this and rhs have the same emptiness-state, width,
    // height, and every pixel of this is almost_equal to the
    // corresponding pixel of rhs.
    bool almost_equal(const same_type& rhs,
		      double delta) const {
      if (empty()) {
	return rhs.empty();
      } else if ((width() != rhs.width()) ||
		 (height() != rhs.height())) {
        return false;
      } else {

        auto rgb_almost_equal = [&](const rgb_type& rgb_l,
				    const rgb_type& rgb_r) {
	  return rgb_l.almost_equal(rgb_r, delta);
	};

        auto row_almost_equal = [&](const row_type& row_l,
				    const row_type& row_r) {
	  return std::equal(row_l.begin(),
			    row_l.end(),
			    row_r.begin(),
			    rgb_almost_equal);
	};

        return std::equal(_rows.begin(),
			  _rows.end(),
			  rhs._rows.begin(),
			  row_almost_equal);
      }
    }

    // Make this image empty.
    void clear() {
      _rows.clear();
      assert(empty());
    }

    // Convert this image to a different color_depth.
    template <typename new_color_depth>
    void convert_to(gfx::image<new_color_depth>& result) const {

      if (empty()) {
	
	result.clear();
	
      } else {

	result.same_size(*this);

	for (int x = 0; x < width(); ++x) {
	  for (int y = 0; y < height(); ++y) {
	    result.pixel(x, y) = pixel(x, y).template convert_to<new_color_depth>();
	  }
	}
      }
    }

    // Return true iff this image is empty.
    bool empty() const {
      return _rows.empty();
    }

    // Return an estimate of the number of bytes used to store pixel
    // data for this image, calculated by
    //
    // width * height * (bytes per pixel) .
    //
    // This is only an estimate and does not include the image<>
    // object overhead nor overhead from the std::vector data
    // structure.
    //
    // When this image is empty, returns 0.
    int estimate_bytes() const {
      if (empty()) {
	return 0;
      } else {
        return width() * height() * sizeof(rgb_type);
      }
    }

    // Overwrite every pixel with default_color.
    void fill(const rgb_type& default_color) {
      for (int y = 0; y < height(); ++y) {
	for (int x = 0; x < width(); ++x) {
	  pixel(x, y) = default_color;
	}
      }
    }

    // Return the height of this image. When the image is empty,
    // returns 0.
    int height() const {
      return _rows.size();
    }

    // Return true iff x is a valid x-coordinate for this image.
    bool is_x(int x) const {
      return !empty() && ((x >= 0) && (x < width()));
    }

    // Return true iff y is a valid y-coordinate for this image.
    bool is_y(int y) const {
      return !empty() && ((y >= 0) && (y < height()));
    }

    // Return a const reference to the pixel at (x, y), which must be
    // valid coordinates. This image must not be empty.
    const rgb_type& pixel(int x, int y) const {
      assert(!empty());
      assert(is_x(x));
      assert(is_y(y));
      return _rows[y][x];
    }

    // Return a mutable reference to the pixel at (x, y), which must
    // be valid coordinates. This image must not be empty.
    rgb_type& pixel(int x, int y) {
      assert(!empty());
      assert(is_x(x));
      assert(is_y(y));
      return _rows[y][x];
    }

    // Change this image's width to new_width, and its height to
    // new_height, both of which must be positive. If these dimensions
    // happen to match the current dimensions, this function has no
    // effect. Otherwise, the pixels at the top-left corner are
    // retained, and any newly-created pixels are initialized to
    // default_color.
    void resize(int new_width,
		int new_height,
		const rgb_type& default_color = BLACK.convert_to<color_depth>()) {

      assert(new_width > 0);
      assert(new_height > 0);

      if ((width() != new_width) || (height() != new_height)) {

	// height
	_rows.resize(new_height, std::vector<rgb_type>(width(), default_color));
	
	// width
	for (auto&& row : _rows) {
	  row.resize(new_width, default_color);
	}
      }

      assert(!empty());
      assert(height() == new_height);
      assert(width() == new_width);
    }

    // Make this image have the same size as other. If other is empty,
    // this function is equivalent to
    //
    //     clear();
    //
    // otherwise it is equivalent to
    //
    //     resize(other.width(), other.height(), default_color);
    // .
    template <typename other_color_depth>
    void same_size(const image<other_color_depth>& other,
		   const rgb_type& default_color = BLACK.convert_to<color_depth>()) {
      if (other.empty()) {
	clear();
      } else {
	resize(other.width(), other.height(), default_color);
      }
    }

    // Swap contents with other.
    void swap(same_type& other) {
      _rows.swap(other._rows);
    }

    // Return the width of this image. When the image is empty,
    // returns 0.
    int width() const {
      if (empty()) {
	return 0;
      } else {
	return _rows[0].size();
      }
    }

  private:

    std::vector<row_type> _rows;
  };

  // Aliases for widely-used color depths.

  using true_color_image = image<true_color_depth>;
  
  using hdr_image = image<hdr_color_depth>;
}
