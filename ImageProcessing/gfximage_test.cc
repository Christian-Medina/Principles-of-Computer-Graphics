///////////////////////////////////////////////////////////////////////////////
// gfximage_test.cc
//
// Unit tests for gfxcolor.hh , gfximage.hh , gfxppm.hh
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdio> // for remove()

#include "rubrictest.hh"

#include "gfxcolor.hh"
#include "gfxfilter.hh"
#include "gfximage.hh"
#include "gfxppm.hh"

int main() {

  Rubric r;

  const bool CREATE_EXAMPLES = false;

  const std::string example_basename("library_"),
    binary_ppm_path = example_basename + "binary.ppm",
    ascii_ppm_path = example_basename + "ascii.ppm",
    clear_component_ppm_path = example_basename + "clear_component.ppm",
    scale_component_ppm_path = example_basename + "scale_component.ppm",
    crop_ppm_path = example_basename + "crop.ppm",
    extend_edges_ppm_path = example_basename + "extended.ppm",
    grayscale_ppm_path = example_basename + "grayscale.ppm",
    edge_detect_ppm_path = example_basename + "edge_detect.ppm",
    box_blur_ppm_path = example_basename + "box_blur.ppm";

  const std::string pattern_box_blur_before_ppm_path = "pattern_box_blur_before.ppm",
    pattern_box_blur_after_ppm_path = "pattern_box_blur_after.ppm";

  if (CREATE_EXAMPLES) {

    gfx::true_color_image before, after;

    bool ok = gfx::ppm_read(before, binary_ppm_path);
    assert(ok);

    clear_component(after, before, gfx::RGB_INDEX_GREEN);
    ok = gfx::ppm_write(after, clear_component_ppm_path);
    assert(ok);

    scale_component(after, before, gfx::RGB_INDEX_BLUE, 1.2);
    ok = gfx::ppm_write(after, scale_component_ppm_path);
    assert(ok);

    crop(after, before, 5, 10, 160, 120);
    ok = gfx::ppm_write(after, crop_ppm_path);
    assert(ok);

    extend_edges(after, before, 20);
    ok = gfx::ppm_write(after, extend_edges_ppm_path);
    assert(ok);

    grayscale(after, before);
    ok = gfx::ppm_write(after, grayscale_ppm_path);
    assert(ok);

    edge_detect(after, before);
    ok = gfx::ppm_write(after, edge_detect_ppm_path);
    assert(ok);

    box_blur(after, before, 5);
    ok = gfx::ppm_write(after, box_blur_ppm_path);
    assert(ok);

    before.resize(80, 80);
    before.fill(gfx::WHITE);
    for (int dy = 0; dy < 10; ++dy) {
      for (int dx = 0; dx < 10; ++dx) {
	before.pixel(20 + dx, 20 + dy) = gfx::RED;
	before.pixel(50 + dx, 20 + dy) = gfx::GREEN;
	before.pixel(20 + dx, 50 + dy) = gfx::BLUE;
	before.pixel(50 + dx, 50 + dy) = gfx::MAROON;
      }
    }
    ok = gfx::ppm_write(before, pattern_box_blur_before_ppm_path);
    assert(ok);
    box_blur(after, before, 3);
    ok = gfx::ppm_write(after, pattern_box_blur_after_ppm_path);
    assert(ok);
  }

  r.criterion("gfxcolor still works",
	      1,
	      [&]() {

                TEST_TRUE("almost_equal handles zero", gfx::almost_equal<float>(0.0, 0.0, .01));
		TEST_TRUE("almost_equal handles near-zero", gfx::almost_equal<float>(0.0001, 0.0, .01));
		TEST_TRUE("almost_equal handles near-zero", gfx::almost_equal<float>(0.0, 0.0001, .01));
		TEST_FALSE("almost_equal handles near-zero", gfx::almost_equal<float>(0.0, 0.1, .01));
		TEST_FALSE("almost_equal handles near-zero", gfx::almost_equal<float>(0.0, 0.1, .01));

                TEST_EQUAL("color_depth::max_value", 255, gfx::true_color_depth::max_value);
		TEST_EQUAL("color_depth::max_value", 1.0, gfx::hdr_color_depth::max_value);

                TEST_EQUAL("color_depth::max_value_int", 255, gfx::true_color_depth::max_value_int);
		TEST_EQUAL("color_depth::max_value_int", 1, gfx::hdr_color_depth::max_value_int);

		TEST_EQUAL("color_depth::max_value_double", 255.0, gfx::true_color_depth::max_value_double);
		TEST_EQUAL("color_depth::max_value_dluble", 1.0, gfx::hdr_color_depth::max_value_double);

		TEST_EQUAL("color_depth::clamp", 0.0, gfx::hdr_color_depth::clamp(-1));
		TEST_EQUAL("color_depth::clamp", 0.0, gfx::hdr_color_depth::clamp(0.0));
		TEST_EQUAL("color_depth::clamp", 1.0, gfx::hdr_color_depth::clamp(1.0));
		TEST_EQUAL("color_depth::clamp", 1.0, gfx::hdr_color_depth::clamp(5.0));

		TEST_FALSE("color_depth::is_value", gfx::hdr_color_depth::is_value(-1.0));
		TEST_TRUE("color_depth::is_value", gfx::hdr_color_depth::is_value(0.0));
		TEST_TRUE("color_depth::is_value", gfx::hdr_color_depth::is_value(0.5));
		TEST_TRUE("color_depth::is_value", gfx::hdr_color_depth::is_value(1.0));
		TEST_FALSE("color_depth::is_value", gfx::hdr_color_depth::is_value(1.5));

		TEST_EQUAL("color_depth::normalize", 0.0, gfx::hdr_color_depth::normalize(0.0));
		TEST_EQUAL("color_depth::normalize", 0.5, gfx::hdr_color_depth::normalize(0.5));
		TEST_EQUAL("color_depth::normalize", 1.0, gfx::hdr_color_depth::normalize(1.0));
		TEST_EQUAL("color_depth::normalize", 0.0, gfx::true_color_depth::normalize(0));
		TEST_EQUAL("color_depth::normalize", 128.0/255.0, gfx::true_color_depth::normalize(128));
		TEST_EQUAL("color_depth::normalize", 1.0, gfx::true_color_depth::normalize(255));

		TEST_EQUAL("color_depth::covert_to", 0, gfx::hdr_color_depth::convert_to<gfx::true_color_depth>(0.0));
		TEST_EQUAL("color_depth::covert_to", 255, gfx::hdr_color_depth::convert_to<gfx::true_color_depth>(1.0));
		TEST_EQUAL("color_depth::covert_to", 0.0, gfx::true_color_depth::convert_to<gfx::hdr_color_depth>(0));
		TEST_EQUAL("color_depth::covert_to", 1.0, gfx::true_color_depth::convert_to<gfx::hdr_color_depth>(255));

		TEST_EQUAL("rgb::rgb()", gfx::true_color_rgb(0, 0, 0), gfx::true_color_rgb());
		TEST_EQUAL("rgb::rgb(rgb&)", gfx::WHITE, gfx::true_color_rgb(gfx::WHITE));
		TEST_EQUAL("rgb::rgb(r,g,b)", (gfx::vector3<uint8_t>{1, 2, 3}), gfx::true_color_rgb(1, 2, 3));

		{
		  gfx::true_color_rgb color(1, 2, 3);
		  TEST_EQUAL("rgb::red", 1, color.red());
		  TEST_EQUAL("rgb::red", 1, color[0]);
		  TEST_EQUAL("rgb::green", 2, color.green());
		  TEST_EQUAL("rgb::green", 2, color[1]);
		  TEST_EQUAL("rgb::blue", 3, color.blue());
		  TEST_EQUAL("rgb::blue", 3, color[2]);

		  color.red() = 11;
		  color.green() = 12;
		  color.blue() = 13;
		  TEST_EQUAL("rgb::red", 11, color.red());
		  TEST_EQUAL("rgb::red", 11, color[0]);
		  TEST_EQUAL("rgb::green", 12, color.green());
		  TEST_EQUAL("rgb::green", 12, color[1]);
		  TEST_EQUAL("rgb::blue", 13, color.blue());
		  TEST_EQUAL("rgb::blue", 13, color[2]);

		  color.assign(21, 22, 23);
		  TEST_EQUAL("rgb::red", 21, color.red());
		  TEST_EQUAL("rgb::red", 21, color[0]);
		  TEST_EQUAL("rgb::green", 22, color.green());
		  TEST_EQUAL("rgb::green", 22, color[1]);
		  TEST_EQUAL("rgb::blue", 23, color.blue());
		  TEST_EQUAL("rgb::blue", 23, color[2]);
		}

		TEST_EQUAL("rgb::convert_to",
			   gfx::true_color_rgb(255, 255, 255),
			   gfx::hdr_rgb(1.0, 1.0, 1.0).template convert_to<gfx::true_color_depth>());
		TEST_EQUAL("rgb::convert_to",
			   gfx::hdr_rgb(1.0, 1.0, 1.0),
			   gfx::true_color_rgb(255, 255, 255).template convert_to<gfx::hdr_color_depth>());

		TEST_EQUAL("color constants", 0x80, gfx::OLIVE.red());
		TEST_EQUAL("color constants", 0x80, gfx::OLIVE.green());
		TEST_EQUAL("color constants", 0x00, gfx::OLIVE.blue());
	      });

  r.criterion("gfximage still works",
	      1,
	      [&]() {

                gfx::true_color_image true_empty, true_blue(100, 200, gfx::BLUE);
		gfx::hdr_image hdr_empty, hdr_black(300, 400);

		TEST_TRUE("image::image()", true_empty.empty());
		TEST_TRUE("image::image()", hdr_empty.empty());

		TEST_FALSE("image::image(int,int)", hdr_black.empty());
		TEST_EQUAL("image::image(int,int)", 300, hdr_black.width());
		TEST_EQUAL("image::image(int,int)", 400, hdr_black.height());

		TEST_FALSE("image::image(int,int,color)", true_blue.empty());
		TEST_EQUAL("image::image(int,int,color)", 100, true_blue.width());
		TEST_EQUAL("image::image(int,int,color)", 200, true_blue.height());

		{
		  gfx::true_color_image lhs;
		  TEST_NOT_EQUAL("image::operator=", lhs, true_blue);
		  lhs = true_blue;
		  TEST_EQUAL("image::operator=", lhs, true_blue);
		}

		TEST_TRUE("image::operator==", true_empty == true_empty);
		TEST_TRUE("image::operator==", true_blue == true_blue);
		TEST_TRUE("image::operator==", true_blue == gfx::true_color_image(true_blue));

		TEST_TRUE("image::operator!=", true_empty != true_blue);
		{
		  auto true_blue_one_white(true_blue);
		  true_blue_one_white.pixel(99, 199) = gfx::WHITE;
		  TEST_TRUE("image::operator!=", true_blue_one_white != true_blue);
		}

		TEST_TRUE("image::almost_equal", true_empty.almost_equal(true_empty, 2));
		TEST_TRUE("image::almost_equal", true_blue.almost_equal(true_blue, 2));
		TEST_TRUE("image::almost_equal", hdr_empty.almost_equal(hdr_empty, .01));
		TEST_TRUE("image::almost_equal", hdr_black.almost_equal(hdr_black, .01));
		TEST_FALSE("image::almost_equal", true_empty.almost_equal(true_blue, 2));
		TEST_FALSE("image::almost_equal", hdr_empty.almost_equal(hdr_black, .01));
		{
		  auto one_dark_gray(hdr_black);
		  one_dark_gray.pixel(299, 399) = gfx::hdr_rgb(0.0, 0.0, .001);
		  TEST_NOT_EQUAL("image::almost_equal", one_dark_gray, hdr_black);
		  TEST_TRUE("image::almost_equal", one_dark_gray.almost_equal(hdr_black, .01));
		}

		{
		  auto temp = hdr_black;
		  TEST_FALSE("image::clear", temp.empty());
		  temp.clear();
		  TEST_TRUE("image::clear", temp.empty());
		}

		{
		  gfx::hdr_image step1;
		  true_blue.convert_to(step1);
		  gfx::true_color_image step2;
		  step1.convert_to(step2);
		  TEST_EQUAL("image::convert_to", step2, true_blue);
		}

		TEST_TRUE("image::empty", true_empty.empty());
		TEST_FALSE("image::empty", true_blue.empty());
		TEST_TRUE("image::empty", hdr_empty.empty());
		TEST_FALSE("image::empty", hdr_black.empty());

		TEST_EQUAL("image::estimate_bytes", 0, true_empty.estimate_bytes());
		TEST_EQUAL("image::estimate_bytes", 100*200*sizeof(gfx::true_color_rgb), true_blue.estimate_bytes());
		TEST_EQUAL("image::estimate_bytes", 300*400*sizeof(gfx::hdr_rgb), hdr_black.estimate_bytes());

		{
		  auto true_red(true_blue);
		  TEST_EQUAL("image::fill", true_red, true_blue);
		  true_red.fill(gfx::RED);
		  TEST_NOT_EQUAL("image::fill", true_red, true_blue);
		  TEST_EQUAL("image::fill", 100, true_red.width());
		  TEST_EQUAL("image::fill", 200, true_red.height());
		  for (int y = 0; y < true_red.height(); ++y) {
		    for (int x = 0; x < true_red.width(); ++x) {
		      TEST_EQUAL("image::fill", true_red.pixel(x, y), gfx::RED);
		    }
		  }
		}

		TEST_EQUAL("image::height", 0, true_empty.height());
		TEST_EQUAL("image::height", 200, true_blue.height());
		TEST_EQUAL("image::height", 0, hdr_empty.height());
		TEST_EQUAL("image::height", 400, hdr_black.height());

		for (int i = -100; i < 300; ++i) {
		  TEST_EQUAL("image::is_x",
			     (i >= 0) && (i < 100),
			     true_blue.is_x(i));
		  TEST_EQUAL("image::is_y",
			     (i >= 0) && (i < 200),
			     true_blue.is_y(i));
		}

		{
		  gfx::true_color_image smaller(true_blue);
		  smaller.resize(10, 20);
		  TEST_EQUAL("image::resize", 10, smaller.width());
		  TEST_EQUAL("image::resize", 20, smaller.height());

		  gfx::true_color_image bigger(true_blue);
		  bigger.resize(300, 400, gfx::WHITE);
		  TEST_EQUAL("image::resize", 300, bigger.width());
		  TEST_EQUAL("image::resize", 400, bigger.height());
		  TEST_EQUAL("image::resize", gfx::WHITE, bigger.pixel(299, 399));
		}

		{
		  // same type
		  gfx::true_color_image temp;
		  TEST_TRUE("image::same_size", temp.empty());
		  temp.same_size(true_blue);
		  TEST_EQUAL("image::same_size", 100, temp.width());
		  TEST_EQUAL("image::same_size", 200, temp.height());

		  // different type
		  temp.same_size(hdr_black);
		  TEST_EQUAL("image::same_size", 300, temp.width());
		  TEST_EQUAL("image::same_size", 400, temp.height());
		}

		{
		  auto a(hdr_empty), b(hdr_black);
		  TEST_EQUAL("image::swap", a, hdr_empty);
		  TEST_EQUAL("image::swap", b, hdr_black);
		  a.swap(b);
		  TEST_EQUAL("image::swap", b, hdr_empty);
		  TEST_EQUAL("image::swap", a, hdr_black);
		}

		TEST_EQUAL("image::width", 0, true_empty.width());
		TEST_EQUAL("image::width", 100, true_blue.width());
		TEST_EQUAL("image::width", 0, hdr_empty.width());
		TEST_EQUAL("image::width", 300, hdr_black.width());
	      });

  r.criterion("gfxppm still works",
	      1,
	      [&]() {
		const std::string temp_path("temp.ppm");

                // read from binary
                gfx::true_color_image from_binary;
		TEST_TRUE("ppm_read from binary",
			  gfx::ppm_read(from_binary, binary_ppm_path));

                // read from ascii
                gfx::true_color_image from_ascii;
		TEST_TRUE("ppm_read from ASCII",
			  gfx::ppm_read(from_ascii, ascii_ppm_path));

		// same result from both files
	        TEST_EQUAL("ppm_read", from_binary, from_ascii);

		// write to binary
		{
		  TEST_TRUE("ppm_write to binary",
			    gfx::ppm_write(from_binary, temp_path));
		  gfx::true_color_image temp;
		  TEST_TRUE("ppm_write to binary", gfx::ppm_read(temp, temp_path));
		  TEST_EQUAL("ppm_write to binary", temp, from_binary);
		  remove(temp_path.c_str());
		}

		// write to ASCII
		{
		  TEST_TRUE("ppm_write to ASCII",
			    gfx::ppm_write(from_binary, temp_path, false));
		  gfx::true_color_image temp;
		  TEST_TRUE("ppm_write to ASCII", gfx::ppm_read(temp, temp_path));
		  TEST_EQUAL("ppm_write to ASCII", temp, from_binary);
		  remove(temp_path.c_str());
		}

		// round trip
		gfx::true_color_image original(400, 300, gfx::MAROON);
		{
		  // binary
		  TEST_TRUE("ppm round trip",
			    gfx::ppm_write(original, temp_path));
		  gfx::true_color_image temp;
		  TEST_TRUE("ppm round trip", gfx::ppm_read(temp, temp_path));
		  TEST_EQUAL("ppm round trip", temp, original);
		  remove(temp_path.c_str());
		}
		{
		  // ASCII
		  TEST_TRUE("ppm round trip",
			    gfx::ppm_write(original, temp_path, false));
		  gfx::true_color_image temp;
		  TEST_TRUE("ppm round trip", gfx::ppm_read(temp, temp_path));
		  TEST_EQUAL("ppm round trip", temp, original);
		  remove(temp_path.c_str());
		}

	      });

  r.criterion("clear_component, scale_component still work",
	      1,
	      [&]() {
                const gfx::true_color_image silver_true(100, 100, gfx::SILVER);
		const gfx::hdr_image silver_hdr(100, 100, gfx::SILVER.convert_to<gfx::hdr_color_depth>());

		// clear_component
		{
		  gfx::true_color_image result_true;
		  gfx::hdr_image result_hdr;
		  clear_component(result_true, silver_true, gfx::RGB_INDEX_GREEN);
		  clear_component(result_hdr, silver_hdr, gfx::RGB_INDEX_GREEN);
		  for (int y = 0; y < result_true.height(); ++y) {
		    for (int x = 0; x < result_true.width(); ++x) {
		      TEST_EQUAL("clear_component<true_color_depth>",
				 gfx::true_color_rgb(0xC0, 0x00, 0xC0),
				 result_true.pixel(x, y));

		      TEST_TRUE("clear_component<hdr_color_depth>",
				gfx::hdr_rgb(192.0/255.0, 0.0, 192.0/255.0).almost_equal(result_hdr.pixel(x, y), .01));
		    }
		  }

		  gfx::true_color_image before, expected, after;
		  TEST_TRUE("clear_component<true_color_depth> : load before",
			    ppm_read(before, binary_ppm_path));
		  TEST_TRUE("clear_component<true_color_depth> : load expected",
			    ppm_read(expected, clear_component_ppm_path));
		  clear_component(after, before, gfx::RGB_INDEX_GREEN);
		  TEST_TRUE("clear_component<true_color_depth> : contents",
			    after.almost_equal(expected, 2));
		}

		// scale_component
		{
		  gfx::true_color_image result_true;
		  gfx::hdr_image result_hdr;
		  scale_component(result_true, silver_true, gfx::RGB_INDEX_GREEN, 1.2);
		  scale_component(result_hdr, silver_hdr, gfx::RGB_INDEX_GREEN, 1.2);
		  for (int y = 0; y < result_true.height(); ++y) {
		    for (int x = 0; x < result_true.width(); ++x) {
		      TEST_TRUE("scale_component<true_color_depth>",
				gfx::true_color_rgb(0xC0, 230, 0xC0).almost_equal(result_true.pixel(x, y), .01));

		      TEST_TRUE("scale_component<hdr_color_depth>",
				gfx::hdr_rgb(192.0/255.0, 230.0/250.0, 192.0/255.0).almost_equal(result_hdr.pixel(x, y), .02));
		    }
		  }
		}
              });

  r.criterion("crop<true_color_depth>",
	      3,
	      [&]() {
                gfx::true_color_image before, expected, after;

		before.clear();
		before.resize(100, 100, gfx::BLUE);
		crop(after, before, 1, 1, 5, 5);
		expected.clear();
		expected.resize(5, 5, gfx::BLUE);
		TEST_TRUE("crop<true_color_depth> : small monochrome image",
			  after.almost_equal(expected, 2));

		TEST_TRUE("crop<true_color_depth> : load before image",
			  gfx::ppm_read(before, binary_ppm_path));

		TEST_TRUE("crop<true_color_depth> : load expected image",
			  gfx::ppm_read(expected, crop_ppm_path));

		crop(after, before, 5, 10, 160, 120);
		TEST_TRUE("crop<true_color_depth> : contents",
			  after.almost_equal(expected, 2));

	      });

  r.criterion("crop<hdr_color_depth>",
	      1,
	      [&]() {
                gfx::hdr_image before, expected, after;
		gfx::true_color_image loaded;

		before.clear();
		before.resize(100, 100, gfx::BLUE.convert_to<gfx::hdr_color_depth>());
		crop(after, before, 1, 1, 5, 5);
		expected.clear();
		expected.resize(5, 5, gfx::BLUE.convert_to<gfx::hdr_color_depth>());
		TEST_TRUE("crop<hdr_color_depth> : small monochrome image",
			  after.almost_equal(expected, .01));

		TEST_TRUE("crop<hdr_color_depth> : load before image",
			  gfx::ppm_read(loaded, binary_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(before);

		TEST_TRUE("crop<hdr_color_depth> : load expected image",
			  gfx::ppm_read(loaded, crop_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(expected);

		crop(after, before, 5, 10, 160, 120);
		TEST_TRUE("crop<hdr_color_depth> : contents",
			  after.almost_equal(expected, .01));

	      });

  r.criterion("extend_edges<true_color_depth>",
	      3,
	      [&]() {
                gfx::true_color_image before, expected, after;

		before.clear();
		before.resize(100, 100, gfx::BLUE);
		extend_edges(after, before, 5);
		expected.clear();
		expected.resize(110, 110, gfx::BLUE);
		TEST_TRUE("extend_edges<true_color_depth> : small monochrome image",
			  after.almost_equal(expected, 2));

		TEST_TRUE("extend_edges<true_color_depth> : load before image",
			  gfx::ppm_read(before, binary_ppm_path));

		TEST_TRUE("extend_edges<true_color_depth> : load expected image",
			  gfx::ppm_read(expected, extend_edges_ppm_path));

		extend_edges(after, before, 20);
		TEST_TRUE("extend_edges<true_color_depth> : contents",
			  after.almost_equal(expected, 2));
	      });

  r.criterion("extend_edges<hdr_color_depth>",
	      1,
	      [&]() {
                gfx::hdr_image before, expected, after;
		gfx::true_color_image loaded;

		before.clear();
		before.resize(100, 100, gfx::BLUE.convert_to<gfx::hdr_color_depth>());
		extend_edges(after, before, 5);
		expected.clear();
		expected.resize(110, 110, gfx::BLUE.convert_to<gfx::hdr_color_depth>());
		TEST_TRUE("extend_edges<hdr_color_depth> : small monochrome image",
			  after.almost_equal(expected, .01));

		TEST_TRUE("extend_edges<hdr_color_depth> : load before image",
			  gfx::ppm_read(loaded, binary_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(before);

		TEST_TRUE("extend_edges<hdr_color_depth> : load expected image",
			  gfx::ppm_read(loaded, extend_edges_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(expected);

		extend_edges(after, before, 20);
		TEST_TRUE("extend_edges<hdr_color_depth> : contents",
			  after.almost_equal(expected, .01));

	      });

  r.criterion("crop_extended_edges<true_color_depth>",
	      3,
	      [&]() {
                gfx::true_color_image before, expected, after;

		before.clear();
		before.resize(100, 100, gfx::BLUE);
		crop_extended_edges(after, before, 5);
		expected.clear();
		expected.resize(90, 90, gfx::BLUE);
		TEST_TRUE("crop_extended_edges<true_color_depth> : small monochrome image",
			  after.almost_equal(expected, 2));

		TEST_TRUE("crop_extended_edges<true_color_depth> : load before image",
			  gfx::ppm_read(before, extend_edges_ppm_path));

		TEST_TRUE("crop_extended_edges<true_color_depth> : load expected image",
			  gfx::ppm_read(expected, binary_ppm_path));

		gfx::crop_extended_edges(after, before, 20);
		TEST_TRUE("crop_extended_edges<true_color_depth> : contents",
			  after.almost_equal(expected, 2));
	      });

  r.criterion("crop_extended_edges<hdr_color_depth>",
	      1,
	      [&]() {
                gfx::hdr_image before, expected, after;
		gfx::true_color_image loaded;

		before.clear();
		before.resize(100, 100, gfx::BLUE.convert_to<gfx::hdr_color_depth>());
		crop_extended_edges(after, before, 5);
		expected.clear();
		expected.resize(90, 90, gfx::BLUE.convert_to<gfx::hdr_color_depth>());
		TEST_TRUE("crop_extended_edges<hdr_color_depth> : small monochrome image",
			  after.almost_equal(expected, .01));

		TEST_TRUE("crop_extended_edges<hdr_color_depth> : load before image",
			  gfx::ppm_read(loaded, extend_edges_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(before);

		TEST_TRUE("crop_extended_edges<hdr_color_depth> : load expected image",
			  gfx::ppm_read(loaded, binary_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(expected);

		gfx::crop_extended_edges(after, before, 20);
		TEST_TRUE("crop_extended_edges<hdr_color_depth> : contents",
			  after.almost_equal(expected, .01));
	      });

        r.criterion("grayscale<true_color_depth>",
      	      3,
      	      [&]() {
                      gfx::true_color_image before, expected, after;

      		TEST_TRUE("grayscale<true_color_depth> : load before image",
      			  gfx::ppm_read(before, binary_ppm_path));

      		TEST_TRUE("grayscale<true_color_depth> : load expected image",
      			  gfx::ppm_read(expected, grayscale_ppm_path));

      		gfx::grayscale(after, before);
      		TEST_TRUE("grayscale<true_color_depth> : contents",
      			  after.almost_equal(expected, 2));
      	      });

        r.criterion("grayscale<hdr_color_depth>",
      	      1,
      	      [&]() {
                      gfx::hdr_image before, expected, after;
      		gfx::true_color_image loaded;

      		TEST_TRUE("grayscale<hdr_color_depth> : load before image",
      			  gfx::ppm_read(loaded, binary_ppm_path));
      		loaded.convert_to<gfx::hdr_color_depth>(before);

      		TEST_TRUE("grayscale<hdr_color_depth> : load expected image",
      			  gfx::ppm_read(loaded, grayscale_ppm_path));
      		loaded.convert_to<gfx::hdr_color_depth>(expected);

      		gfx::grayscale(after, before);
      		TEST_TRUE("grayscale<hdr_color_depth> : contents",
      			  after.almost_equal(expected, .01));
      	      });

  r.criterion("edge_detect<true_color_depth>",
	      3,
	      [&]() {
                gfx::true_color_image before, expected, after;

		TEST_TRUE("edge_detect<true_color_depth> : load before image",
			  gfx::ppm_read(before, binary_ppm_path));

		TEST_TRUE("edge_detect<true_color_depth> : load expected image",
			  gfx::ppm_read(expected, edge_detect_ppm_path));

		gfx::edge_detect(after, before);
		TEST_TRUE("edge_detect<true_color_depth> : contents",
			  after.almost_equal(expected, 2));
	      });

  r.criterion("edge_detect<hdr_color_depth>",
	      1,
	      [&]() {
                gfx::hdr_image before, expected, after;
		gfx::true_color_image loaded;

		TEST_TRUE("edge_detect<hdr_color_depth> : load before image",
			  gfx::ppm_read(loaded, binary_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(before);

		TEST_TRUE("edge_detect<hdr_color_depth> : load expected image",
			  gfx::ppm_read(loaded, edge_detect_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(expected);

		gfx::edge_detect(after, before);
		TEST_TRUE("edge_detect<hdr_color_depth> : contents",
			  after.almost_equal(expected, .01));
	      });

  r.criterion("box_blur<true_color_depth>",
	      3,
	      [&]() {
                gfx::true_color_image before, expected, after;

		TEST_TRUE("box_blur<true_color_depth> : load before image",
			  gfx::ppm_read(before, pattern_box_blur_before_ppm_path));

		TEST_TRUE("box_blur<true_color_depth> : load expected image",
			  gfx::ppm_read(expected, pattern_box_blur_after_ppm_path));

		gfx::box_blur(after, before, 3);
		TEST_TRUE("box_blur<true_color_depth> : contents",
			  after.almost_equal(expected, 2));

		TEST_TRUE("box_blur<true_color_depth> : load before image",
			  gfx::ppm_read(before, binary_ppm_path));

		TEST_TRUE("box_blur<true_color_depth> : load expected image",
			  gfx::ppm_read(expected, box_blur_ppm_path));

		gfx::box_blur(after, before, 5);
		TEST_TRUE("box_blur<true_color_depth> : contents",
			  after.almost_equal(expected, 2));
	      });

  r.criterion("box_blur<hdr_color_depth>",
	      1,
	      [&]() {
                gfx::hdr_image before, expected, after;
		gfx::true_color_image loaded;

		TEST_TRUE("box_blur<hdr_color_depth> : load before image",
			  gfx::ppm_read(loaded, pattern_box_blur_before_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(before);

		TEST_TRUE("box_blur<hdr_color_depth> : load expected image",
			  gfx::ppm_read(loaded, pattern_box_blur_after_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(expected);

		gfx::box_blur(after, before, 3);
		TEST_TRUE("box_blur<hdr_color_depth> : contents",
			  after.almost_equal(expected, .01));

		TEST_TRUE("box_blur<hdr_color_depth> : load before image",
			  gfx::ppm_read(loaded, binary_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(before);

		TEST_TRUE("box_blur<hdr_color_depth> : load expected image",
			  gfx::ppm_read(loaded, box_blur_ppm_path));
		loaded.convert_to<gfx::hdr_color_depth>(expected);

		gfx::box_blur(after, before, 5);
		TEST_TRUE("box_blur<hdr_color_depth> : contents",
			  after.almost_equal(expected, .01));
	      });

  return r.run();
}
