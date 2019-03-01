///////////////////////////////////////////////////////////////////////////////
// raytrace_demo.cc
//
// Demonstration of the ray tracer code declared in gfxraytrace.hh .
//
// This is a program with a main() that creates 9 images:
//
// - A single green triangle against a black background; drawn with
//   all four combinations of flat/Blinn-Phong shanding and
//   orthographic/perspective project.
//
// - Two spheres of radius 1.0, one red and close to the camera, and
//   one blue and farther away. This scene is rendered in the same
//   four configurations as the previous bullet item.
//
// - The Utah Teapot, drawn in fuscia against a gray background, with
//   perspective and Blinn-Phong shading.
//
///////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <sstream>

#include "gfxcolor.hh"
#include "gfxppm.hh"
#include "gfxraytrace.hh"

const gfx::hdr_rgb HDR_WHITE = gfx::WHITE.convert_to<gfx::hdr_color_depth>(),
                   HDR_BLACK = gfx::BLACK.convert_to<gfx::hdr_color_depth>();

int main() {

  std::unique_ptr<gfx::camera> the_camera;  
  std::unique_ptr<gfx::viewport> the_viewport;
  std::unique_ptr<gfx::abstract_projection> the_projection;
  std::unique_ptr<gfx::abstract_shader> the_shader;
  
  // Create a raytracer object that will be used for the scenes that
  // get rendered 4 ways each.
  {
    the_camera.reset(new gfx::camera(gfx::vector3<double>{0.0, 0.0, 0.0},
				     gfx::vector3<double>{0.0, 0.0, 1.0},
				     gfx::vector3<double>{0.0, -1.0, 0.0}));
    
    the_viewport.reset(new gfx::viewport(400, 400, -1, +1, +1, -1));

    // Note: the_projection and the_shader are still null.
    gfx::raytracer rt(the_camera, the_viewport, the_projection, the_shader);
    assert(!rt.ready());

    // Loop through all combinations of scene, ortho/projection, and
    // flat/Blinn-Phong.
    for (int triangle = 0; triangle <= 1; ++triangle) {

      gfx::scene the_scene(gfx::BLACK.convert_to<gfx::hdr_color_depth>());

      // Create lights.
      the_scene.add_light(gfx::vector3<double>{-2.0, 2.0, -1.0}, HDR_WHITE);
      the_scene.add_light(gfx::vector3<double>{+1.0, 0.0, -1.0},
			  gfx::hdr_rgb(.25, .25, .25));

      // Decide how to add the scene objects.
      if (triangle) {
	// scene is one triangle

	std::unique_ptr<gfx::abstract_scene_object> lime_triangle;
	lime_triangle.reset(new gfx::scene_triangle(gfx::LIME.convert_to<gfx::hdr_color_depth>(),
						    8.0,
						    gfx::vector3<double>{-.3, .3, 2.0},
						    gfx::vector3<double>{+.3, .3, 2.0},
						    gfx::vector3<double>{0.0, -.3, 2.0}));
	the_scene.add_object(lime_triangle);

      } else {
	// scene is two spheres

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
      }

      // Now render 4 different ways.
      for (int ortho = 0; ortho <= 1; ++ortho) {
	for (int flat = 0; flat <= 1; ++flat) {

	  // create PPM filename
	  std::stringstream path;
	  path << (triangle ? "triangle" : "spheres") << "_"
	       << (ortho ? "ortho" : "persp") << "_"
	       << (flat ? "flat" : "phong")
	       << ".ppm";
	  const std::string ppm_path(path.str());

	  std::cerr << ppm_path << "...";
	  
	  // create projection
	  if (ortho) {
	    the_projection.reset(new gfx::orthographic_projection());
	  } else {
	    the_projection.reset(new gfx::perspective_projection(1.0));
	  }
	  rt.set_projection(the_projection);

	  // create shader
	  if (flat) {
	    the_shader.reset(new gfx::flat_shader());
	  } else {
	    the_shader.reset(new gfx::blinn_phong_shader(0.0, // ambient coeff
							 HDR_WHITE, // ambient color
							 1.0, // diffuse coeff
							 0.5)); // specular coeff
	  }
	  rt.set_shader(the_shader);

	  assert(rt.ready());

	  // render in HDR
	  gfx::hdr_image hdr_render;
	  rt.render(hdr_render, the_scene);

	  // convert to true color so we can write to PPM
	  gfx::true_color_image true_color;
	  hdr_render.convert_to<gfx::true_color_depth>(true_color);

	  // write PPM file
	  bool ok = ppm_write(true_color, path.str());
	  assert(ok);

	  std::cerr << "\n";
	}
      }
    }
  }

  // Now the teapot scene, which is only rendered one way.
  {
    const std::string ppm_path("utah-teapot.ppm");
    std::cerr << ppm_path << "...";
    
    // Create raytracer.
    the_camera.reset(new gfx::camera(gfx::vector3<double>{-200.0, 100.0, 100.0},
				     gfx::vector3<double>{0.0, -1.0, 0.0},
				     gfx::vector3<double>{0.0, 0.0, -1.0}));
    the_viewport.reset(new gfx::viewport(400, 400, -1, +1, +1, -1));
    the_projection.reset(new gfx::perspective_projection(1.0));
    the_shader.reset(new gfx::blinn_phong_shader(0.0, HDR_WHITE, 1.0, 0.5));
    gfx::raytracer rt(the_camera, the_viewport, the_projection, the_shader);

    // Create scene.
    gfx::scene the_scene(gfx::hdr_rgb{.90, .90, .90});
    the_scene.add_light(gfx::vector3<double>{-300, 200, 0.0}, HDR_WHITE);
    the_scene.add_light(gfx::vector3<double>{-100, 0, 0.0}, gfx::hdr_rgb{.10, .10, .10});

    // Load teapot geometry.
    std::vector<std::array<gfx::vector3<double>, 3>> teapot;
    bool ok = obj_read(teapot, "utah-teapot.obj");
    assert(ok);
    assert(!teapot.empty());
    for (auto&& face : teapot) {
      std::unique_ptr<gfx::abstract_scene_object> triangle;
      triangle.reset(new gfx::scene_triangle(gfx::FUSCIA.convert_to<gfx::hdr_color_depth>(),
					     16.0,
					     face[0],
					     face[1],
					     face[2]));
      the_scene.add_object(triangle);
    }

    // render in HDR
    gfx::hdr_image hdr_render;
    rt.render(hdr_render, the_scene);

    // convert to true color so we can write to PPM
    gfx::true_color_image true_color;
    hdr_render.convert_to<gfx::true_color_depth>(true_color);

    // write PPM file
    ppm_write(true_color, "utah-teapot.ppm");

    std::cerr << "\n";
  }
  
  return 0;
}
