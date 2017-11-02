#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
{
//Initialisierung der Planeten nach Struct
  planet sun = {"sun",0.2,0.0,0.8};
  planet mercury = {"mercury",1.0,1.5,0.1};
  planet venus = {"venus",0.9,2.0,0.15};
  planet earth = {"earth",0.8,2.5,0.15};
  planet mars = {"mars",0.7,3.5,0.2};
  planet jupiter = {"jupiter",0.6,4.5,0.3};
  planet saturn = {"saturn",0.5,4.9,0.3};
  planet uranus = {"uranus",0.4,5.0,0.25};
  planet neptune = {"neptune",0.3,5.5,0.25};
//Befüllen des planets-Vectors
  planets.insert(planets.end(),{sun,mercury,venus,earth,mars,jupiter,saturn,uranus,neptune});
  initializeGeometry();
  initializeShaderPrograms();
}

void ApplicationSolar::upload_planet_transforms(planet p) const {
//Matrixtransformationen unter Verwendung der Werte der Planeten
  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime()*p.rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*p.distance});
  model_matrix = glm::scale(model_matrix, glm::fvec3{1.0f*p.size, 1.0f*p.size, 1.0f*p.size});
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));
  }

void ApplicationSolar::upload_moon_transforms(moon m) const {
//Suche nach passendem Planeten für den Mond
  planet p;
  for(std::vector<planet>::size_type i = 0; i != planets.size(); i++) {
     p = planets.at(i);
     if(p.name == m.o_name){
      break;
     }
}
//Transformationen der Erde
  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime()*p.rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*p.distance});
//Transformationen des Mondes im Bezug zur Erde
  model_matrix = glm::rotate(model_matrix, float(glfwGetTime()*m.rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*m.distance});
  model_matrix = glm::scale(model_matrix, glm::fvec3{1.0f*m.size, 1.0f*m.size, 1.0f*m.size});
  
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));
  }
void ApplicationSolar::render() const {

  glUseProgram(m_shaders.at("planet").handle);

//Schleife über alle Planeten im Vector
  for(std::vector<planet>::size_type i = 0; i != planets.size(); i++) {
    planet p = planets.at(i);
    upload_planet_transforms(p);

  glBindVertexArray(planet_object.vertex_AO);

  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }
//Initalisierung des Erdenmondes nach Moon-Struct
  moon e_moon = {"e_moon",5.0,0.5,0.08,"earth"};
  upload_moon_transforms(e_moon);

  glBindVertexArray(planet_object.vertex_AO);

  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

}

void ApplicationSolar::updateView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::updateProjection() {
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() {
  updateUniformLocations();
  
  // bind new shader
  glUseProgram(m_shaders.at("planet").handle);
  
  updateView();
  updateProjection();
}

// handle key input
void ApplicationSolar::keyCallback(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.1f});
    updateView();
  }
  else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.1f});
    updateView();
  }
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  // mouse handling
}

// load shader programs
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", "star", shader_program{m_resource_path + "shaders/simple.vert",
                                           m_resource_path + "shaders/simple.frag"});
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;

  m_shaders.at("star").u_locs["NormalMatrix"] = 1;
  m_shaders.at("star").u_locs["ViewMatrix"] = 1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = 1;
}

// load models
void ApplicationSolar::initializeGeometry() {
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);
 // model star adden
  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);

   // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

  // store type of primitive to draw
  planet_object.draw_mode = GL_TRIANGLES;
  // transfer number of indices to model object 
  planet_object.num_elements = GLsizei(planet_model.indices.size());
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
}

// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<ApplicationSolar>(argc, argv);
}