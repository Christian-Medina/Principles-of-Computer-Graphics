///////////////////////////////////////////////////////////////////////////////
// gfxmath.hh
//
// Representation of an RGB color. This includes a color_depth
// template class that defines a color encoding; and an
// rgb<color_depth_type> template class that defines one
// (red, green, blue) color.
//
// This module builds on gfxmath.hh, so familiarize yourself with that
// file before using this one.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>

#include "gfxmath.hh"

namespace gfx {

  // A color_depth is an encoding scheme for a color channel intensity
  // (i.e. one of red intensity, green intensity, or blue intensity),
  // defined by a numerical data type to store the intensity (probably
  // int, uint8_t, float, or double) and a constant value of that type
  // defining the maximum possible intensity (probably 255 or 1.0).
  template <typename component_type_parameter, int max_value_int_parameter>
  class color_depth {
  public:

    // Type aliases.
    using component_type = component_type_parameter;
    static constexpr component_type max_value = max_value_int_parameter;
    static constexpr int max_value_int = max_value_int_parameter;
    static constexpr double max_value_double = max_value_int_parameter;

    // Return
    //
    // x            if 0 <= x <= max_value
    // 0            if x < 0
    // max_value    if x > max_value
    static component_type clamp(component_type x) {
      if (x < 0) {
	return 0;
      } else if (x > max_value) {
        return max_value;
      } else {
        return x;
      }
    }

    // Return true iff x is a valid intensity value, i.e.
    // 0 <= x <= max_value .
    static constexpr bool is_value(component_type x) {
      return (x >= 0) && (x <= max_value);
    }

    // Return x normalized to the range [0, 1].
    static double normalize(component_type x) {
      return static_cast<double>(x) / max_value_double;
    }

    // Convert this intensity value to an equivalent intensity from
    // another color depth.
    template <typename other_color_depth>
    static typename other_color_depth::component_type convert_to(component_type x) {
      double out_of_1 = normalize(x),
	out_of_other_max = out_of_1 * other_color_depth::max_value_double;
      return static_cast<typename other_color_depth::component_type>(out_of_other_max);
    }
  };

  // Aliases for widely-used color depths.
  
  using true_color_depth = color_depth<uint8_t, 255>;

  using hdr_color_depth = color_depth<float, 1>;

  // An rgb_index identifies one of the red, green, or blue color channels.
  enum rgb_index { RGB_INDEX_RED   = 0,
		   RGB_INDEX_GREEN = 1,
		   RGB_INDEX_BLUE  = 2 };

  // Return true iff i is a valid rgb_index .
  bool is_rgb_index(int i) {
    return (i >= 0) && (i <= 2);
  }

  // rgb represents a (red, green, blue) tuple encoded in a specific
  // color depth. Mathematically this is a 3-vector, so rgb is a
  // subclass of vector3.  Therefore it inherits convenient vector3
  // members such as the [] and arithmetic operators. rgb adds
  // functions specific to 3-vectors that represent colors.
  template <typename color_depth_type_parameter>
  class rgb : public gfx::vector3<typename color_depth_type_parameter::component_type> {
  public:

    // Type aliases.
    using color_depth_type = color_depth_type_parameter;
    using same_type = rgb<color_depth_type>;
    using color_component_type = typename color_depth_type::component_type;
    using parent_type = gfx::vector3<color_component_type>;

    // Default constructor, intializes all components to zero.
    rgb()
      : parent_type{0, 0, 0} { }

    rgb(const same_type& rhs)
      : parent_type(rhs) { }

    // Constructor that uses passed-in red, green, blue intensities.
    rgb(color_component_type r,
	color_component_type g,
	color_component_type b)
      : parent_type{r, g, b} {
      assert(color_depth_type::is_value(r));
      assert(color_depth_type::is_value(g));
      assert(color_depth_type::is_value(b));
    }

    // Aliases to access the three components by name.
    const color_component_type& red() const {
      return (*this)[RGB_INDEX_RED];
    }
    color_component_type& red() {
      return (*this)[RGB_INDEX_RED];
    }
    const color_component_type& green() const {
      return (*this)[RGB_INDEX_GREEN];
    }
    color_component_type& green() {
      return (*this)[RGB_INDEX_GREEN];
    }
    const color_component_type& blue() const {
      return (*this)[RGB_INDEX_BLUE];
    }
    color_component_type& blue() {
      return (*this)[RGB_INDEX_BLUE];
    }

    // Assign the three intensity values to r, g, b.
    void assign(color_component_type r,
		color_component_type g,
		color_component_type b) {

      assert(color_depth_type::is_value(r));
      assert(color_depth_type::is_value(g));
      assert(color_depth_type::is_value(b));
      
      red() = r;
      green() = g;
      blue() = b;
    }

    // Convert this rgb to an argb of another color_depth type.
    template <typename other_color_depth>
    rgb<other_color_depth> convert_to() const {
      auto xvert = color_depth_type::template convert_to<other_color_depth>;
      return rgb<other_color_depth>(xvert(red()),
				    xvert(green()),
				    xvert(blue()));
    }

  private:

    std::array<color_component_type, 3> _components;
  };

  // Aliases for widely-used color depths.

  using true_color_rgb = gfx::rgb<true_color_depth>;

  using hdr_rgb = gfx::rgb<hdr_color_depth>;

  // Function to convert a 24-bit hexadecimal HTML color code into a
  // true_color_rgb object.
  true_color_rgb hex_color(int hex) {
    assert(hex >= 0);
    assert(hex <= 0xFFFFFF);
    int r = (hex >> 16) & 0xFF,
        g = (hex >>  8) & 0xFF,
        b = hex         & 0xFF;
    return true_color_rgb(r, g, b);
  }

  // Constant colors corresponding to the HTML color names.
  // https://en.wikipedia.org/wiki/Web_colors#HTML_color_names
  const true_color_rgb AQUA   = hex_color(0x00FFFF),
                       BLACK  = hex_color(0x000000),
                       BLUE   = hex_color(0x0000FF),
                       FUSCIA = hex_color(0xFF00FF),
                       GRAY   = hex_color(0x808080),
                       GREEN  = hex_color(0x008000),
                       LIME   = hex_color(0x00FF00),
                       MAROON = hex_color(0x800000),
                       NAVY   = hex_color(0x000080),
                       OLIVE  = hex_color(0x808000),
                       PURPLE = hex_color(0x800080),
                       RED    = hex_color(0xFF0000),
                       SILVER = hex_color(0xC0C0C0),
                       TEAL   = hex_color(0x008080),
                       WHITE  = hex_color(0xFFFFFF),
                       YELLOW = hex_color(0xFFFF00);

}
