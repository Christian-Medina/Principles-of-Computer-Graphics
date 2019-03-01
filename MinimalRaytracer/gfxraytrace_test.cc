///////////////////////////////////////////////////////////////////////////////
// gfxraytrace_test.cc
//
// Unit tests for gfxraytrace.hh
//
///////////////////////////////////////////////////////////////////////////////

#include "rubrictest.hh"

#include "gfxppm.hh"
#include "gfxraytrace.hh"

int main() {
  
  Rubric r;

  const double DELTA = .03;
  
  const gfx::hdr_rgb BACKGROUND_COLOR = gfx::TEAL.template convert_to<gfx::hdr_color_depth>(),
    HDR_WHITE = gfx::WHITE.template convert_to<gfx::hdr_color_depth>();

  const std::string expected_triangle_path("expected_triangle.ppm"),
    expected_spheres_path("expected_spheres.ppm");
  
  r.criterion("raytrace empty scene",
	      3,
	      [&]() {

		std::unique_ptr<gfx::camera> the_camera(new gfx::camera({0, 0, 0},
									{0, 0, -1},
									{0, 1, 0}));

		std::unique_ptr<gfx::viewport> the_viewport(new gfx::viewport(400,
									      300,
									      -1, 1,
									      1, -1));
		std::unique_ptr<gfx::abstract_projection> the_projection(new gfx::orthographic_projection());
		std::unique_ptr<gfx::abstract_shader> the_shader(new gfx::flat_shader());

		gfx::raytracer rt(the_camera, the_viewport, the_projection, the_shader);

		TEST_TRUE("raytracer constructor", rt.ready());

		// deliberately empty
		gfx::scene the_scene(BACKGROUND_COLOR);
		
		gfx::hdr_image result;
		rt.render(result, the_scene);

		TEST_FALSE("result not empty", result.empty());
		TEST_EQUAL("result width", 400, result.width());
		TEST_EQUAL("result height", 300, result.height());

		for (int y = 0; y < result.height(); ++y) {
		  for (int x = 0; x < result.width(); ++x) {
		    TEST_TRUE("background color R",
			      gfx::almost_equal(result.pixel(x, y).red(),
						BACKGROUND_COLOR.red(),
						DELTA));
		    TEST_TRUE("background color G",
			      gfx::almost_equal(result.pixel(x, y).green(),
						BACKGROUND_COLOR.green(),
						DELTA));
		    TEST_TRUE("background color B",
			      gfx::almost_equal(result.pixel(x, y).blue(),
						BACKGROUND_COLOR.blue(),
						DELTA));
		  }
		}
	      });

  r.criterion("spheres as shown in spheres_ortho_flat.ppm",
	      3,
	      [&]() {

		std::unique_ptr<gfx::camera> the_camera;  
		std::unique_ptr<gfx::viewport> the_viewport;
		std::unique_ptr<gfx::abstract_projection> the_projection;
		std::unique_ptr<gfx::abstract_shader> the_shader;
  		the_camera.reset(new gfx::camera(gfx::vector3<double>{0.0, 0.0, 0.0},
						 gfx::vector3<double>{0.0, 0.0, 1.0},
						 gfx::vector3<double>{0.0, -1.0, 0.0}));
		the_viewport.reset(new gfx::viewport(400, 400, -1, +1, +1, -1));
		gfx::raytracer rt(the_camera, the_viewport, the_projection, the_shader);
		gfx::scene the_scene(gfx::BLACK.convert_to<gfx::hdr_color_depth>());
		the_scene.add_light(gfx::vector3<double>{-2.0, 2.0, -1.0}, HDR_WHITE);
		the_scene.add_light(gfx::vector3<double>{+1.0, 0.0, -1.0},
				    gfx::hdr_rgb(.25, .25, .25));
		std::unique_ptr<gfx::abstract_scene_object> red_sphere;
		red_sphere.reset(new gfx::scene_sphere(gfx::RED.convert_to<gfx::hdr_color_depth>(),
						       4.0,
						       gfx::vector3<double>{-1.0, 0.0, 2.0},
						       0.5));
		the_scene.add_object(red_sphere);
		std::unique_ptr<gfx::abstract_scene_object> blue_sphere;
		red_sphere.reset(new gfx::scene_sphere(gfx::BLUE.convert_to<gfx::hdr_color_depth>(),
						       4.0,
						       gfx::vector3<double>{1.0, 0.0, 8.0},
						       0.5));
		the_scene.add_object(red_sphere);
		the_projection.reset(new gfx::orthographic_projection());
		rt.set_projection(the_projection);
		the_shader.reset(new gfx::flat_shader());
		rt.set_shader(the_shader);
		assert(rt.ready());
		gfx::hdr_image got_hdr;
		rt.render(got_hdr, the_scene);
		gfx::true_color_image expected_true_color;
		TEST_TRUE("load expected image",
			  gfx::ppm_read(expected_true_color, expected_spheres_path));
		gfx::hdr_image expected_hdr;
		expected_true_color.convert_to<gfx::hdr_color_depth>(expected_hdr);

		TEST_TRUE("image almost equal",
			  expected_hdr.almost_equal(got_hdr, DELTA));
	      });
  
  r.criterion("triangles as shown in triangle_ortho_flat.ppm",
	      3,
	      [&]() {

		std::unique_ptr<gfx::camera> the_camera;  
		std::unique_ptr<gfx::viewport> the_viewport;
		std::unique_ptr<gfx::abstract_projection> the_projection;
		std::unique_ptr<gfx::abstract_shader> the_shader;
  		the_camera.reset(new gfx::camera(gfx::vector3<double>{0.0, 0.0, 0.0},
						 gfx::vector3<double>{0.0, 0.0, 1.0},
						 gfx::vector3<double>{0.0, -1.0, 0.0}));
		the_viewport.reset(new gfx::viewport(400, 400, -1, +1, +1, -1));
		gfx::raytracer rt(the_camera, the_viewport, the_projection, the_shader);
		gfx::scene the_scene(gfx::BLACK.convert_to<gfx::hdr_color_depth>());
		the_scene.add_light(gfx::vector3<double>{-2.0, 2.0, -1.0}, HDR_WHITE);
		the_scene.add_light(gfx::vector3<double>{+1.0, 0.0, -1.0},
				    gfx::hdr_rgb(.25, .25, .25));
		std::unique_ptr<gfx::abstract_scene_object> lime_triangle;
		lime_triangle.reset(new gfx::scene_triangle(gfx::LIME.convert_to<gfx::hdr_color_depth>(),
							    8.0,
							    gfx::vector3<double>{-.3, .3, 2.0},
							    gfx::vector3<double>{+.3, .3, 2.0},
							    gfx::vector3<double>{0.0, -.3, 2.0}));
		the_scene.add_object(lime_triangle);
		the_projection.reset(new gfx::orthographic_projection());
		rt.set_projection(the_projection);
		the_shader.reset(new gfx::flat_shader());
		rt.set_shader(the_shader);
		assert(rt.ready());
		gfx::hdr_image got_hdr;
		rt.render(got_hdr, the_scene);
		gfx::true_color_image expected_true_color;
		TEST_TRUE("load expected image",
			  gfx::ppm_read(expected_true_color, expected_triangle_path));
		gfx::hdr_image expected_hdr;
		expected_true_color.convert_to<gfx::hdr_color_depth>(expected_hdr);

		TEST_TRUE("image almost equal",
			  expected_hdr.almost_equal(got_hdr, DELTA));
	      });
  
  return r.run();
}

