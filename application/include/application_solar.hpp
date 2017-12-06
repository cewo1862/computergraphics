#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"

// gpu representation of model
class ApplicationSolar : public Application {
 public:
  // allocate and initialize objects
  ApplicationSolar(std::string const& resource_path);
  // free allocated objects
  ~ApplicationSolar();

  // update uniform locations and values
  void uploadUniforms();
  // update projection matrix
  void updateProjection();
  // react to key input
  void keyCallback(int key, int scancode, int action, int mods);
  //handle delta mouse movement input
  void mouseCallback(double pos_x, double pos_y);

  void upload_planet_transforms(planet p) const;
  void upload_sky_transforms(planet p) const;
  void upload_moon_transforms(moon m) const;
  void upload_orbit_transforms(planet p) const;
  void upload_orbit_transforms(moon m) const;
  void generate_stars();
  void generate_orbits();


  // draw all objects
  void render() const;

 protected:
  void initializeShaderPrograms();
  void initializeGeometry();
  void initializeTextures();
  void updateView();

  // cpu representation of model
  model_object planet_object;
  model_object star_object;
  model_object orbit_object;
  std::vector<planet> planets;
  std::vector<float> svalues;
  std::vector<float> ovalues;
};

#endif