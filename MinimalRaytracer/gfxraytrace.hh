///////////////////////////////////////////////////////////////////////////////
// gfxraytrace.hh
//
// Minimalist ray tracer.
//
// This header includes the following classes, which are interrelated:
//
// - raytracer represents the complete raytracing process, and
//   composes the following classes, each of which is responsible for
//   a smaller part of raytracing:
//
//       - camera defines the viewer's location and orientation
//
//       - viewport defines the dimensions of the view window in both
//         world coordintes and sceen coordinates
//
//       - abstract_projection is an abstract class for a projection,
//         which can generate view rays
//
//           - orthographic_projection is an abstract_projection that
//             performs orthographic projection, with parallel view
//             rays
//
//           - perspective_projection is an abstract_projection that
//             performs perspective-correct projection, where all view
//             rays originate from the camera location
//
//       - abstract_shader is an abstract class for a shader, which
//         computes the color of a pixel based on a ray-object
//         itersection
//
//           - flat shader just passes object color through and does
//             not take lighting into account
//
//           - blinn_phong_shader uses the Blinn-Phong illumination
//             model to combine ambient light, diffuse light, and
//             specular highlights
//
// - view_ray is a viewing ray, with an origin and direction
//
// - abstract_scene_object is an abstract geometric scene object
//
//       - scene_sphere is a 3D sphere
//
//       - scene_triangle is a 3D triangle
//
// - point_light is a light located at a specific coordinate
//
// - intersection represents an intersection between a view ray and a
//   scene object
//
// - scene represents a scene that may be rendered, including lights,
//   scene objects, and a background color
//
// In addition, the header includes one function:
//
// - obj_read is a crude reader for the Wavefront .OBJ file format
//   that stores a geometric model as a triangle mesh. This function
//   is intended only to load the utah-teapot.obj file.
//
// This module builds on gfxmath.hh and gfximage.hh, so familiarize
// yourself with those files before using this one.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#include "gfxmath.hh"
#include "gfximage.hh"

namespace gfx {

  // Forward declarations of new class types, in alphabetical order.
  
  class abstract_projection;
  class abstract_scene_object;
  class abstract_shader;
  class blinn_phong_shader;
  class camera;
  class flat_shader;
  class intersection;
  class orthographic_projection;
  class perspective_projection;
  class point_light;
  class raytracer;
  class scene;
  class scene_sphere;
  class scene_triangle;
  class view_ray;
  class viewport;

  // Class declarations, in the order that classes are introduced in
  // the comment above.

  // A raytracer object composes everything that is necessary to
  // render an image. We are using OOP composition here to break what
  // would otherwise be an overwhelming amount of code, into smaller
  // digestable classes.
  //
  // A raytracer owns one each of: camera, viewport, projection, and
  // shader. It manages these objects through std::unique_ptr's. When
  // all four pointers are initialized and non-null, the raytracer is
  // "ready". The interesting member functions only work when the
  // raytracer is "ready".
  class raytracer {
  public:

    // Default constructor, creates a raytracer without any camera,
    // viewport, projection, or shader. The raytracer is not ready.
    raytracer() {
      assert( !ready() );
    }

    // Construct a raytracer with given camera, viewport, projection,
    // and shader. If all are non-null then the raytracer will be
    // ready.
    raytracer(std::unique_ptr<camera>& the_camera,
	      std::unique_ptr<viewport>& the_viewport,
	      std::unique_ptr<abstract_projection>& the_projection,
	      std::unique_ptr<abstract_shader>& the_shader)
      : _camera(std::move(the_camera)),
	_viewport(std::move(the_viewport)),
	_projection(std::move(the_projection)),
	_shader(std::move(the_shader)) { }

    // Return true when the raytracer is ready, i.e. all four pointers
    // are non-null.
    bool ready() const {
      return (has_camera() &&
	      has_viewport() &&
	      has_projection() &&
	      has_shader());
    }

    // Return true when the raytracer has a non-null camera.
    bool has_camera() const {
      return bool(_camera);
    }

    // Return true when the raytracer has a non-null viewport.
    bool has_viewport() const {
      return bool(_viewport);
    }

    // Return true when the raytracer has a non-null projection.
    bool has_projection() const {
      return bool(_projection);
    }

    // Return true when the raytracer has a non-null shader.
    bool has_shader() const {
      return bool(_shader);
    }

    // Get the current camera, which must be non-null.
    const camera& the_camera() const {
      assert(has_camera());
      return *_camera;
    }

    // Get the current viewport, which must be non-null.
    const viewport& the_viewport() const {
      assert(has_viewport());
      return *_viewport;
    }

    // Get the current projection, which must be non-null.
    const abstract_projection& projection() const {
      assert(has_projection());
      return *_projection;
    }

    // Get the current shader, which must be non-null.
    const abstract_shader& shader() const {
      assert(has_shader());
      return *_shader;
    }

    // Accessors which are equivalent to the four previous ones, but
    // return non-const references.
    class camera& the_camera() {
      return const_cast<class camera&>(static_cast<const raytracer&>(*this).the_camera());
    }
    class viewport& the_viewport() {
      return const_cast<class viewport&>(static_cast<const raytracer&>(*this).the_viewport());
    }
    class abstract_projection& projection() {
      return const_cast<abstract_projection&>(static_cast<const raytracer&>(*this).projection());
    }
    abstract_shader& shader() {
      return const_cast<abstract_shader&>(static_cast<const raytracer&>(*this).shader());
    }

    // Mutators
    void set_camera(std::unique_ptr<camera>& the_camera) {
      _camera = std::move(the_camera);
    }
    void set_viewport(std::unique_ptr<viewport>& the_viewport) {
      _viewport = std::move(the_viewport);
    }
    void set_projection(std::unique_ptr<abstract_projection>& the_projection) {
      _projection = std::move(the_projection);
    }
    void set_shader(std::unique_ptr<abstract_shader>& the_shader) {
      _shader = std::move(the_shader);
    }
    
    // Render the given scene, and place the output image in
    // result. This function may only be called when is_ready() is
    // true.
    void render(hdr_image& result,
		const scene& scene) const;

  private:

    std::unique_ptr<camera> _camera;
    std::unique_ptr<viewport> _viewport;
    std::unique_ptr<abstract_projection> _projection;
    std::unique_ptr<abstract_shader> _shader;
  };

  // The location and orientation of the camera. This is defined by a
  // 3D point for the eye location; and a basis defined by three 3D
  // normalized vectors.
  class camera {
  public:

    // Constructor that provides the eye location and three basis
    // vectors directly. u, v, and w must each be normalized (length
    // 1.0).
    camera(const vector3<double>& eye,
	   const vector3<double>& u,
	   const vector3<double>& v,
	   const vector3<double>& w)
      : _eye(eye),
	_u(u),
	_v(v),
	_w(w) { }

    // Constructor that computes the basis in terms of a given
    // view-direction and up vector.
    camera(const vector3<double>& eye,
	   const vector3<double>& view_direction,
	   const vector3<double>& up);

    // Accessors.
    const vector3<double>& eye() const {
      return _eye;
    }
    const vector3<double>& u() const {
      return _u;
    }
    const vector3<double>& v() const {
      return _v;
    }
    const vector3<double>& w() const {
      return _w;
    }
    
  private:

    vector3<double> _eye, _u, _v, _w;
  };

  // A viewport defines the boundary of the viewing window. It stores
  // the width and height of the image, in screen coordinates; and the
  // left, right, top, and bottom of the view window in world
  // coordinates.
  class viewport {
  public:

    // Constructor. The following inequalities must hold:
    //
    // x_resolution, y_resolution > 0
    // left < 0 < right
    // bottom < 0 < top
    //
    viewport(int x_resolution,
	     int y_resolution,
	     double left,
	     double top,
	     double right,
	     double bottom)
      : _x_resolution(x_resolution),
	_y_resolution(y_resolution),
	_left(left),
	_top(top),
	_right(right),
	_bottom(bottom) {

      assert(x_resolution > 0);
      assert(y_resolution > 0);
      assert(left < 0.0);
      assert(right > 0.0);
      assert(top > 0.0);
      assert(bottom < 0.0);
    }

    // Accessors.
    int x_resolution() const {
      return _x_resolution;
    }
    int y_resolution() const {
      return _y_resolution;
    }
    double left() const {
      return _left;
    }
    double top() const {
      return _top;
    }
    double right() const {
      return _right;
    }
    double bottom() const {
      return _bottom;
    }

    // Map an (x, y) screen coordinate to a (u, v) coordinate in
    // [0, 1]^2. The output (u, v) are passed by reference.
    void uv(double& u,
	    double& v,
	    int x,
	    int y) const;
    
  private:

    int _x_resolution, _y_resolution;
    double _left, _top, _right, _bottom;
  };

  // Abstract class defining a projection algorithm.
  class abstract_projection {
  public:

    // Given a camera and (u, v) coordinate within the viewport,
    // create a viewing ray and store it in result. (u, v) are
    // expected to come from the camera::uv function.
    virtual void compute_view_ray(view_ray& result,
				  const camera& c,
				  double u,
				  double v) const = 0;
  };
  
  // Orthographic implementation of abstract_projection.
  class orthographic_projection : public abstract_projection {
  public:

    orthographic_projection() { }

    virtual void compute_view_ray(view_ray& result,
				  const camera& c,
				  double u,
				  double v) const;
  };

  // Perspective implementation of abstract_projection.
  class perspective_projection : public abstract_projection {
  public:

    // The perspective projection algorithm needs to know the
    // focal_length of the camera, which is the distance between the
    // eye and the view plane, and must be positive.
    perspective_projection(double focal_length)
      : _focal_length(focal_length) {

      assert(focal_length > 0.0);
    }

    // Accessor.
    double focal_length() const {
      return _focal_length;
    }

    virtual void compute_view_ray(view_ray& result,
				  const camera& c,
				  double u,
				  double v) const;
    
  private:
    
    double _focal_length;
  };

  // Abstract class defining a shading algorithm.
  class abstract_shader {
  public:

    // Given a scene, camera, and particular ray-object intersection,
    // compute the color of the pixel corresponding to the view
    // ray. The pixel's color is returned.
    virtual hdr_rgb shade(const scene& scene,
			  const camera& camera,
			  const intersection& xsect) const = 0;
  };

  // Flat-shader implementation of abstract_shader.
  class flat_shader : public abstract_shader {
  public:
    
    virtual hdr_rgb shade(const scene& scene,
			  const camera& camera,
			  const intersection& xsect) const;
  };

  // Blin-Phong implementation of abstract_shader.
  class blinn_phong_shader : public abstract_shader {
  public:

    // The Blinn-Phong model depends on the following parameters:
    //
    // ambient_coefficient is the multiplier for ambient light, which
    // must be non-negative. When zero there will be no ambient light.
    // 
    // ambient_color is the color of ambient light; usually white in
    // daylight.
    //
    // diffuse_coefficient is the multiplier for diffuse light (object
    // color), which must be non-negative. When zero there is no
    // diffuse light, so only ambient and specular light would be
    // visible.
    //
    // specular_coefficient is the multiplier for specular light
    // (speckles/gloss/glare), which must be non-negative. When zero
    // there are no specular highlights so all objects appear matte.
    //
    blinn_phong_shader(double ambient_coefficient,
		       const hdr_rgb& ambient_color,
		       double diffuse_coefficient,
		       double specular_coefficient)
      : _ambient_coefficient(ambient_coefficient),
	_ambient_color(ambient_color),
	_diffuse_coefficient(diffuse_coefficient),
	_specular_coefficient(specular_coefficient) {

      assert(ambient_coefficient >= 0.0);
      assert(diffuse_coefficient >= 0.0);
      assert(specular_coefficient >= 0.0);
    }

    // Accessors.
    double ambient_coefficient() const {
      return _ambient_coefficient;
    }
    const hdr_rgb& ambient_color() const {
      return _ambient_color;
    }
    double diffuse_coefficient() const {
      return _diffuse_coefficient;
    }
    double specular_coefficient() const {
      return _specular_coefficient;
    }
    
    virtual hdr_rgb shade(const scene& scene,
			  const camera& camera,
			  const intersection& xsect) const;

  private:

    double _ambient_coefficient;
    hdr_rgb _ambient_color;
    double _diffuse_coefficient, _specular_coefficient;
  };

  // A view ray represents a ray traveling from the viewer out into
  // the scene. It is defined by an origin, and direction, each of
  // which is a 3D vector.
  class view_ray {
  public:

    // Default constructor that sets origin and direction both to all
    // zero.
    view_ray()
      : _origin(0.0),
	_direction(0.0) { }

    // Constructor with an explicit origin and direction. Direction
    // must be normalized (magnitude 1).
    view_ray(const vector3<double>& origin,
	     const vector3<double>& direction)
      : _origin(origin),
	_direction(direction) { }

    // Accessors.
    const vector3<double>& origin() const {
      return _origin;
    }
    const vector3<double>& direction() const {
      return _direction;
    }

    // Convenience function to assign both the origin and direciton.
    void assign(const vector3<double>& origin,
		const vector3<double>& direction) {
      _origin = origin;
      _direction = direction;
    }
    
  private:

    vector3<double> _origin, _direction;
  };

  // Abstract class for some kind of scene object.
  class abstract_scene_object {
  public:

    // Construct an object with the given diffuse color and shininesss
    // value (Phong exponent). shininess must be positive.
    abstract_scene_object(const hdr_rgb& color,
			  double shininess)
      : _color(color),
	_shininess(shininess) {

      assert(shininess > 0.0);
    }

    // Accessors.
    const hdr_rgb& color() const {
      return _color;
    }
    double shininess() const {
      return _shininess;
    }

    // Virtual function to find the intersection between this object
    // and the given viewing ray, if any.
    //
    // This function determines whether there exists an intersection
    // with ray, for some t such that
    //         t_min <= t < t_upper_bound .
    // Note that t must be strictly less than t_upper_bound.
    //
    // t_min must be less than t_upper_bound. t_upper_bound may be
    // floating-point infinity.
    //
    // If no such intersection exists, return false.
    //
    // If there is an intersection within that t range, update result
    // to reflect the details of the intersection, and return true.
    virtual bool intersect(intersection& result,
			   const view_ray& ray,
			   double t_min,
			   double t_upper_bound) const = 0;


  private:

    hdr_rgb _color;
    double _shininess;
  };

  // A scene object that is a 3D sphere.
  class scene_sphere : public abstract_scene_object {
  public:

    // Create a sphere with the given color, shininess, center
    // location, and radius. radius must be positive.
    scene_sphere(const hdr_rgb& color,
		 double shininess,
		 const vector3<double>& center,
		 double radius)
      : abstract_scene_object(color, shininess),
	_center(center),
	_radius(radius) {

       assert(radius > 0.0);
     }

    // Accessors.
    const vector3<double>& center() const {
      return _center;
    }
    double radius() const {
      return _radius;
    }

    virtual bool intersect(intersection& result,
			   const view_ray& ray,
			   double t_min,
			   double t_upper_bound) const;

  private:
    vector3<double> _center;
    double _radius;
  };

  // A scene object that is a 3D triangle.
  class scene_triangle : public abstract_scene_object {
  public:

    // The three vertices of the triangle are called a, b, c. Each is
    // a 3D location.
    scene_triangle(const hdr_rgb& color,
		   double shininess,
		   const vector3<double>& a,
		   const vector3<double>& b,
		   const vector3<double>& c)
      : abstract_scene_object(color, shininess),
	_a(a),
	_b(b),
	_c(c) { }

    // Accessors.
    const vector3<double>& a() const {
      return _a;
    }
    const vector3<double>& b() const {
      return _b;
    }
    const vector3<double>& c() const {
      return _c;
    }
    
    virtual bool intersect(intersection& result,
			   const view_ray& ray,
			   double t_min,
			   double t_upper_bound) const;

  private:

    vector3<double> _a, _b, _c;
  };

  // A point_light represents a light source that gives off the same
  // amount of light in all directions. The sun, or an ideal light
  // bulb, can be modeled as a point light.
  class point_light {
  public:

    // Construct a point light at the given location, that emits light
    // of the given color. To make the light dimmer, use smaller
    // intensity values in the color.
    point_light(const vector3<double>& location,
		const hdr_rgb& color)
      : _location(location),
	_color(color) { }

    // Accessors.
    const vector3<double>& location() const {
      return _location;
    }
    const hdr_rgb& color() const {
      return _color;
    }

  private:
    
    vector3<double> _location;
    hdr_rgb _color;
  };

  // An intersection represents a place where a view ray hits a
  // scene object. It is defined by:
  //
  // - a non-owning pointer to the object that was hit;
  //
  // - the 3D point where the hit occurs;
  //
  // - a normal vector, that is perpendicular to the object at the hit
  //   location; and
  //
  // - the t value where the hit happened relative to the view ray
  //   direction, i.e.
  //
  //       location == ray.origin + (t * ray.direction)
  //
  // When all of these values have been initialized, the intersection
  // object is "ready" and all its member functions work. When member
  // variables have not been initialized, the intersection is "not
  // ready" and some member functions cannot work.
  class intersection {
  public:

    // Construct an intersection that is not ready. The object pointer
    // is initialized to null and the other values are initialized to
    // zero.
    intersection()
      : _object(nullptr),
	_location(0.0),
	_normal(0.0),
	_t(0.0) {

      assert( !ready());
    }

    // Construct a ready intersection. object must be non-null, and t
    // must be non-negative.
    intersection(const abstract_scene_object* object,
		 const vector3<double>& location,
		 const vector3<double>& normal,
		 double t)
      : _object(object),
	_location(location),
	_normal(normal),
	_t(t) {

      assert(object);
      assert(t >= 0.0);

      assert(ready());
    }

    // Return when this interesection is "ready," i.e. all its member
    // variables have been initialized.
    bool ready() const {
      return bool(_object);
    }

    // Assign all member variables, making this intersection object
    // "ready." object must be non-null and t must be non-negative.
    void assign(const abstract_scene_object* object,
		const vector3<double>& location,
		const vector3<double>& normal,
		double t) {

      assert(object);
      assert(t >= 0.0);

      _object = object;
      _location = location;
      _normal = normal;
      _t = t;

      assert(ready());
    }

    // Accessors.
    const abstract_scene_object& object() const {
      assert(ready());
      return *_object;
    }
    const vector3<double>& location() const {
      assert(ready());
      return _location;
    }
    const vector3<double>& normal() const {
      assert(ready());
      return _normal;
    }
    double t() const {
      assert(ready());
      return _t;
    }
    
  private:

    const abstract_scene_object *_object; // non-owning pointer
    vector3<double> _location, _normal;
    double _t;
  };

  // A scene represents all the geometric information necessary to
  // render an image. That includes:
  //
  // - a background color, used to fill a pixel whose view ray does
  //   not intersect any scene object;
  //
  // - a vector of point lights; and
  //
  // - a vector of scene objects.
  //
  // Ordinarily you need at least one light, and many scene objects,
  // to make an interesting image. However this is not enforced with
  // assertions.
  class scene {
  public:

    // Constructor.
    scene(const hdr_rgb& background)
      : _background(background) { }

    // Accessors.
    const hdr_rgb& background() const {
      return _background;
    }

    const std::vector<std::unique_ptr<point_light>>& lights() const {
      return _lights;
    }

    const std::vector<std::unique_ptr<abstract_scene_object>>& objects() const {
      return _objects;
    }

    // Mutators.
    void set_background(const hdr_rgb& background) {
      _background = background;
    }
    void add_light(std::unique_ptr<point_light>& light);
    void add_light(const vector3<double>& location,
		   const hdr_rgb& color);
    void add_object(std::unique_ptr<abstract_scene_object>& object);
    void clear_lights() {
      _lights.clear();
    }
    void clear_objects() {
      _objects.clear();
    }
    void clear() {
      clear_lights();
      clear_objects();
    }

    // Trace a ray and find the first intersecting scene object. ray
    // is the view ray to trace; t_min and t_upper_bound define a
    // range of acceptable t values, defined the same way as in
    // abstract_scene_object::intersect.
    //
    // When no such intersection exists, return false.
    //
    // When some intersection exists within the given t range, assign
    // the details into result, and return true.
    bool ray_trace(intersection& result,
		   const view_ray& ray,
		   double t_min,
		   double t_upper_bound) const;

  private:

    hdr_rgb _background;
    std::vector<std::unique_ptr<point_light>> _lights;
    std::vector<std::unique_ptr<abstract_scene_object>> _objects;
  };

// Read a Wavefront .OBJ file, containing triangle geometry, into
// result. This function is a quick hack intended only to load
// utah-teapot.obj. The function does not intend or claim to implement
// the .OBJ standard perfectly, or work for other .obj files. On I/O
// or parse error, return false. Otherwise each element of result
// represents one triangle, and each triangle is defined by three 3D
// points, and the function returns true.
bool obj_read(std::vector<std::array<gfx::vector3<double>, 3>>& result,
	      const std::string& path) {

  std::ifstream f(path);
  if (!f) {
    return false;
  }

  std::vector<gfx::vector3<double>> vertices;
  std::vector<std::array<int, 3>> faces;

  for (std::string line; getline(f, line);) {
    if (line.empty()) {
      continue;
    }

    switch (line[0]) {
    case 'v': // vertex
      {
	std::string should_be_v;
	double x, y, z;
	std::stringstream ss(line);
	ss >> should_be_v >> x >> y >> z;
	if ((should_be_v != "v") || !ss) {
	  return false;
	}
	vertices.push_back({x, y, z});
	break;
      }

    case 'f':
      {
	std::string should_be_f;
	int i, j, k;
	std::stringstream ss(line);
	ss >> should_be_f >> i >> j >> k;
	if ((should_be_f != "f") || !ss) {
	  return false;
	}
	faces.push_back({i, j, k});
	break;
      }

    default:
      break;
    }
  }

  f.close();
  
  if (vertices.empty() || faces.empty()) {
    return false;
  }

  result.clear();
  
  for (auto&& face : faces) {
    int i = face[0] - 1,
        j = face[1] - 1,
        k = face[2] - 1;
    result.push_back({vertices[i], vertices[j], vertices[k]});
  }

  return true;
}
  
void raytracer::render(hdr_image& result,
		       const scene& scene) const {

  assert(ready());
  
  int w = _viewport->x_resolution(),
      h = _viewport->y_resolution();
      
  result.resize(w, h);

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {

      // compute viewing ray for each pixel. If object is hit, compute n,
	  //evalueate shading model, and set pixel to corresponding color.
	  //Else set pixel to background color.
	  
	  double u = 0;
	  double v = 0;
	  view_ray rayOne;
	  bool bHit = false;
	  intersection sect;
	  _viewport->uv(u,v,x,y);
	  _projection->compute_view_ray(rayOne,the_camera(),u,v);
	  bHit=scene.ray_trace(sect,rayOne,0,std::numeric_limits<double>::infinity());
	  
	  if(bHit)
	  {
		  result.pixel(x,y) = _shader->shade(scene,the_camera(),sect);
	  }
	  else
	  {
		  result.pixel(x,y) = scene.background();
	  }
	  
    }
  }
}

camera::camera(const vector3<double>& eye,
	       const vector3<double>& view_direction,
	       const vector3<double>& up)
  : _eye(eye) {
		_w = (-(view_direction.normalize()));
		_u = ((up.cross(w())).normalize());
		_v = ((w().cross(u())).normalize());
}

void viewport::uv(double& u,
		  double& v,
		  int x,
		  int y) const {
  
		u = (left()+((right()-left())*(x+0.5))/x_resolution());
		v = (bottom()+((top()-bottom())*(y+0.5))/y_resolution());
}

void orthographic_projection::compute_view_ray(view_ray& result,
					       const camera& c,
					       double u,
					       double v) const {

		result.assign((((c.eye())+(c.u()*u))+(c.v()*(v))),(c.w()*-1));
}

void perspective_projection::compute_view_ray(view_ray& result,
					      const camera& c,
					      double u,
					      double v) const {

		vector3<double> test = c.w()*-1;
		result.assign(c.eye(), (test*(_focal_length)));
}

hdr_rgb flat_shader::shade(const scene& scene,
			   const camera& camera,
			   const intersection& xsect) const {
  

  
  return (xsect.object().color().convert_to<gfx::hdr_color_depth>());
}

hdr_rgb blinn_phong_shader::shade(const scene& scene,
				  const camera& camera,
				  const intersection& xsect) const {
	hdr_rgb L;
	L.red() = ambient_color().red()*ambient_coefficient();
	L.blue() = ambient_color().blue()*ambient_coefficient();
	L.green() = ambient_color().green()*ambient_coefficient();
	auto n = xsect.normal();
	double Kd = diffuse_coefficient();
	double Ks = specular_coefficient();
	int p = xsect.object().shininess();
	for(int i = 0; i < scene.lights().size(); i++)
	{
		auto l = (scene.lights()[i]->location() - xsect.location());
		l = l.normalize();
		auto v = (camera.eye()-xsect.location());
		v = v.normalize();
		auto h = (l+v);
		h = h.normalize();
		auto nh = (n.normalize()*h.normalize());
		
		L.red() += (Kd*std::max(0.0, (n * l)) + (Ks * (pow(std::max(0.0, nh),p))));
		L.blue() += (Kd*std::max(0.0, (n * l)) + (Ks * (pow(std::max(0.0, nh),p))));
		L.green() += (Kd*std::max(0.0, (n * l)) + (Ks * (pow(std::max(0.0, double(nh)),p))));
	}
	
	if((L.red() > float(255)) > 1)
    {
      L.red() = float(255);
    }
	if((L.green() > float(255)) > 1)
    {
      L.green() = float(255);
    }
	if((L.blue() > float(255)) > 1)
    {
      L.blue() = float(255);
    }
	
  return L;
}

bool scene_sphere::intersect(intersection& result,
			     const view_ray& ray,
			     double t_min,
			     double t_upper_bound) const {

  assert(t_min < t_upper_bound);
      
  double t;
  vector3<double> normal;
  const vector3<double> dif = this->center()-ray.origin();
  double a = ray.direction()*ray.direction();
  double b = 2*(ray.direction()*dif);
  double c = dif*dif-pow(this->radius(),2);
  
  double discrim = (b*b-4*a*c);
  
  if(discrim < 0)
  {
	  return false;
  }
  else
  {
	  double tOne = (sqrt(discrim)+ -b)/(2*a);
	  double tTwo = (-sqrt(discrim)+ -b)/(2*a);
	  if((tOne<=tTwo) && (tOne>=0))
	  {
		  t = tOne;
		  normal = dif*2;
		  result.assign(this,dif,normal,t);
		  return true;
	  }
	  else if(tTwo>=0)
	  {
		  t = tTwo;
		  normal = dif*2;
		  result.assign(this,dif,normal,t);
		  return true;
	  }
	  else
	  {
		  return false;
	  }
  }

}

bool scene_triangle::intersect(intersection& result,
			       const view_ray& ray,
			       double t_min,
			       double t_upper_bound) const {

  assert(t_min < t_upper_bound);

  // TODO: Fill in the body of this function, then delete these
  // skeleton comments.
  //
  // Hint: This process is described very precisely in section
  // 4.4.2.
  //
  // You can use the gfx::matrix::solve function you implemented in
  // project 1. The textbook writes out how to use Cramer's rule here,
  // and it's OK to follow those instructions, but it's easier and
  // more concise to just call gfx::matrix::solve.
  //
  // After you compute the t, gamma, and beta values corresponding to
  // the intersection, make sure that you compare gamma and beta
  // precisely as described in the pseudocode on the bottom of page
  // 79.
  return false;
}
  
bool scene::ray_trace(intersection& result,
		      const view_ray& ray,
		      double t_min,
		      double t_upper_bound) const {

  assert(t_min < t_upper_bound);

  bool hit = false;
  auto hitObj = result;
  
  for(int i = 0; i < _objects.size(); i++)
  {
	  if((_objects[i])->intersect(result,ray,t_min,t_upper_bound))
	  {
		  hit = true;
		  hitObj = result;
		  t_upper_bound = result.t();
	  }
	  
	  result = hitObj;
	  return hit;
  }
  
  return false;
}

void scene::add_light(std::unique_ptr<point_light>& light) {
  assert(light);
  _lights.push_back(std::move(light));
}

void scene::add_light(const vector3<double>& location,
		      const hdr_rgb& color) {
  auto light = std::unique_ptr<point_light>(new point_light(location, color));
  add_light(light);
}

void scene::add_object(std::unique_ptr<abstract_scene_object>& object) {
  assert(object);
  _objects.push_back(std::move(object));
}

}
