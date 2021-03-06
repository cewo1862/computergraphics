#include "application_solar.hpp"
#include "launcher.hpp"
#include <cstdlib>

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"

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
  planet sun = {"sun",0.2,0.0,0.8,0.5,0.5,0.5, texture_object{}};
  planet mercury = {"mercury",1.0,1.5,0.1,0.25,0.25,0.15, texture_object{}};
  planet venus = {"venus",0.9,2.0,0.15,0.5,0.25,0.5, texture_object{}};
  planet earth = {"earth",0.8,2.5,0.15,0.75,0.75,0.25, texture_object{}};
  planet mars = {"mars",0.7,3.5,0.2,1.0,0.7,0.5, texture_object{}};
  planet jupiter = {"jupiter",0.6,4.5,0.3,0.9,0.6,0.35, texture_object{}};
  planet saturn = {"saturn",0.5,4.9,0.3,0.125,0.12,0.8, texture_object{}};
  planet uranus = {"uranus",0.4,5.0,0.25,0.12,0.6,0.134, texture_object{}};
  planet neptune = {"neptune",0.3,5.5,0.25,1.0,0.8,0.5, texture_object{}};
  planet sky = {"sky",0.0,0.0,1.0,0.0,0.0,0.0, texture_object{}};
//Befüllen des planets-Vectors
  planets.insert(planets.end(),{sky,sun,mercury,venus,earth,mars,jupiter,saturn,uranus,neptune});
  generate_orbits();
  generate_stars();
  generate_quad();
  initializeGeometry();
  initializeFrameBuffer();
  initializeTextures();
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
  glUniform3f(m_shaders.at("planet").u_locs.at("in_Color"),p.r_color,p.g_color,p.b_color);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D,p.p_texture.handle);
  int color_sampler_location = glGetUniformLocation(m_shaders.at("planet").handle, "ColorTex");
  glUseProgram(m_shaders.at("planet").handle);
  glUniform1i(color_sampler_location, 1);
}

void ApplicationSolar::upload_sky_transforms(planet p) const {
  
  glDepthMask(GL_FALSE);

  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime()*p.rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*p.distance});
  model_matrix = glm::scale(model_matrix, glm::fvec3{1.0f*p.size, 1.0f*p.size, 1.0f*p.size});
  glUniformMatrix4fv(m_shaders.at("sky").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at("sky").u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));
  glUniform3f(m_shaders.at("sky").u_locs.at("in_Color"),p.r_color,p.g_color,p.b_color);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D,p.p_texture.handle);
  int color_sampler_location = glGetUniformLocation(m_shaders.at("sky").handle, "ColorTex");
  glUseProgram(m_shaders.at("sky").handle);
  glUniform1i(color_sampler_location, 1);

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
  glUniform3f(m_shaders.at("planet").u_locs.at("in_Color"),m.r_color,m.g_color,m.b_color);
}

void ApplicationSolar::upload_orbit_transforms(planet p) const{
  glUseProgram(m_shaders.at("orbit").handle);
  glm::fmat4 model_matrix = glm::scale(glm::fmat4{}, glm::fvec3{p.distance, 0.0f, p.distance});
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));
}

void ApplicationSolar::upload_orbit_transforms(moon m) const{
  glUseProgram(m_shaders.at("orbit").handle);
  planet p;
  for(std::vector<planet>::size_type i = 0; i != planets.size(); i++) {
     p = planets.at(i);
     if(p.name == m.o_name){
      break;
     }
 }

  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime()*p.rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*p.distance});
  model_matrix = glm::scale(model_matrix, glm::fvec3{m.distance, 0.0f, m.distance});
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));
}

void ApplicationSolar::upload_quad_transforms() const{
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D,t_texture.handle);
  int color_sampler_location = glGetUniformLocation(m_shaders.at("quad").handle, "ColorTex");
  glUseProgram(m_shaders.at("quad").handle);
  glUniform1i(color_sampler_location, 1);


  glUseProgram(m_shaders.at("quad").handle);

  glBindVertexArray(quad_object.vertex_AO); 
  
  glDrawArrays(quad_object.draw_mode, NULL , quad_object.num_elements);

}

void ApplicationSolar::generate_stars() {
  int number_of_stars = 1000;
  for(int i=1;i<=number_of_stars; i++){
    svalues.push_back((std::rand()%100)-50);
    svalues.push_back((std::rand()%100)-50);
    svalues.push_back((std::rand()%100)-50);
    svalues.push_back((std::rand()%100)-50);
    svalues.push_back((std::rand()%100)-50);
    svalues.push_back((std::rand()%100)-50);
  }
}

void ApplicationSolar::generate_orbits() {
  for(int i=0;i<360;i++){
    ovalues.push_back(sin(i*M_PI/180));
    ovalues.push_back(0);
    ovalues.push_back(cos(i*M_PI/180));
  }
}

void ApplicationSolar::generate_quad() {
    qvalues.push_back(-1.0);
    qvalues.push_back(-1.0);
    qvalues.push_back(0.0);
    qvalues.push_back(0.0);
    qvalues.push_back(0.0);

    qvalues.push_back(1.0);
    qvalues.push_back(-1.0);
    qvalues.push_back(0.0);
    qvalues.push_back(1.0);
    qvalues.push_back(0.0);

    qvalues.push_back(-1.0);
    qvalues.push_back(1.0);
    qvalues.push_back(0.0);
    qvalues.push_back(0.0);
    qvalues.push_back(1.0);

    qvalues.push_back(1.0);
    qvalues.push_back(1.0);
    qvalues.push_back(0.0);
    qvalues.push_back(1.0);
    qvalues.push_back(1.0);
}

void ApplicationSolar::render() const {

  glBindFramebuffer(GL_FRAMEBUFFER, fb_texture.handle);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//Schleife über alle Planeten im Vector
  for(std::vector<planet>::size_type i = 0; i != planets.size(); i++) {

  planet p = planets.at(i);

  if(p.name == "sky"){
    glUseProgram(m_shaders.at("sky").handle);

    upload_sky_transforms(p);

    glBindVertexArray(planet_object.vertex_AO);

    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

    glDepthMask(1);
  }

  else{

  glUseProgram(m_shaders.at("planet").handle);

  upload_planet_transforms(p);

  glBindVertexArray(planet_object.vertex_AO);

  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

  upload_orbit_transforms(p);

  glBindVertexArray(orbit_object.vertex_AO);

  glDrawArrays(orbit_object.draw_mode, NULL , orbit_object.num_elements);   
  }
}
//Initalisierung des Erdenmondes nach Moon-Struct
  moon e_moon = {"e_moon",5.0,0.5,0.08,"earth",0.1,0.4,0.35};
  glUseProgram(m_shaders.at("planet").handle);
  upload_moon_transforms(e_moon);

  glBindVertexArray(planet_object.vertex_AO);

  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

  upload_orbit_transforms(e_moon);

  glBindVertexArray(orbit_object.vertex_AO);

  glDrawArrays(orbit_object.draw_mode, NULL , orbit_object.num_elements);

  glUseProgram(m_shaders.at("star").handle);

  glBindVertexArray(star_object.vertex_AO); 
  
  glDrawArrays(star_object.draw_mode, NULL , star_object.num_elements);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  upload_quad_transforms();
}

void ApplicationSolar::updateView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);

  glUseProgram(m_shaders.at("planet").handle);
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("sky").handle);
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("sky").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("star").handle);


  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("orbit").handle);


  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::updateProjection() {
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);

  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  // upload matrix to gpu
  glUseProgram(m_shaders.at("sky").handle);

  glUniformMatrix4fv(m_shaders.at("sky").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("star").handle);


  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("orbit").handle);


  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glBindRenderbuffer(GL_RENDERBUFFER, rb_texture.handle);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w_width, w_height);

  glBindTexture(GL_TEXTURE_2D, t_texture.handle);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w_width,w_height,
                0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
}

// update uniform locations
void ApplicationSolar::uploadUniforms() {
  updateUniformLocations();
  
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
   else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{-0.1f, 0.0f, 0.0f});
    updateView();
  }
   else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.1f, 0.0f, 0.0f});
    updateView();
  }
  else if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
    if(hori_switch == 0){
    hori_switch = 1;
    }
    else{
    hori_switch = 0;
    }
    int hori_switch_location = glGetUniformLocation(m_shaders.at("quad").handle, "hori_switch");
    glUseProgram(m_shaders.at("quad").handle);
    glUniform1i(hori_switch_location, hori_switch);

  }
  else if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
    if(vert_switch == 0){
    vert_switch = 1;
    }
    else{
    vert_switch = 0;
    }
    int vert_switch_location = glGetUniformLocation(m_shaders.at("quad").handle, "vert_switch");
    glUseProgram(m_shaders.at("quad").handle);
    glUniform1i(vert_switch_location, vert_switch);
  }
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  // mouse handling
    m_view_transform = glm::rotate(m_view_transform,0.03f, glm::fvec3{pos_x,pos_y, 0.0f});
    updateView();
}

// load shader programs
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/planet.vert",
                                           m_resource_path + "shaders/planet.frag"});
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["in_Color"] = -1;
  m_shaders.at("planet").u_locs["ColorTex"] = -1;

  m_shaders.emplace("sky", shader_program{m_resource_path + "shaders/sky.vert",
                                           m_resource_path + "shaders/sky.frag"});
  // request uniform locations for shader program
  m_shaders.at("sky").u_locs["NormalMatrix"] = -1;
  m_shaders.at("sky").u_locs["ModelMatrix"] = -1;
  m_shaders.at("sky").u_locs["ViewMatrix"] = -1;
  m_shaders.at("sky").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("sky").u_locs["in_Color"] = -1;
  m_shaders.at("sky").u_locs["ColorTex"] = -1;

  m_shaders.emplace("star", shader_program{m_resource_path + "shaders/star.vert",
                                           m_resource_path + "shaders/star.frag"});
  // request uniform locations for shader program
  m_shaders.at("star").u_locs["ViewMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;

  m_shaders.emplace("orbit", shader_program{m_resource_path + "shaders/orbit.vert",
                                           m_resource_path + "shaders/orbit.frag"});
  
  m_shaders.at("orbit").u_locs["ModelMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ViewMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ProjectionMatrix"] = -1;

  m_shaders.emplace("quad", shader_program{m_resource_path + "shaders/quad.vert",
                                           m_resource_path + "shaders/quad.frag"});
  m_shaders.at("quad").u_locs["ColorTex"] = -1;
  m_shaders.at("quad").u_locs["vert_switch"] = -1;
  m_shaders.at("quad").u_locs["hori_switch"] = -1;

}
void ApplicationSolar::initializeGeometry() {
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", (model::NORMAL | model::TEXCOORD));
  model star_model =  model{svalues, model::NORMAL+model::POSITION, {}};
  model orbit_model =  model{ovalues, model::POSITION, {}};
  model quad_model = model{qvalues, model::POSITION+model::TEXCOORD, {}};
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

  glEnableVertexAttribArray(2);

  glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);

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

  // generate vertex array object
  glGenVertexArrays(1, &star_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(star_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &star_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * star_model.data.size(), star_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::NORMAL]);

  // store type of primitive to draw
  star_object.draw_mode = GL_POINTS;
  // transfer number of indices to model object 
  star_object.num_elements = GLsizei(svalues.size()/6);


  // generate vertex array object
  glGenVertexArrays(1, &orbit_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(orbit_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &orbit_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, orbit_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * orbit_model.data.size(), orbit_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, orbit_model.vertex_bytes, orbit_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  
  // store type of primitive to draw
  orbit_object.draw_mode = GL_LINE_LOOP;
  // transfer number of indices to model object 
  orbit_object.num_elements = GLsizei(ovalues.size()/3);


  // generate vertex array object
  glGenVertexArrays(1, &quad_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(quad_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &quad_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, quad_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * quad_model.data.size(), quad_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, quad_model.vertex_bytes, quad_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, quad_model.vertex_bytes, quad_model.offsets[model::TEXCOORD]);

  // store type of primitive to draw
  quad_object.draw_mode = GL_TRIANGLE_STRIP;
  // transfer number of indices to model object 
  quad_object.num_elements = GLsizei(qvalues.size()/5);



  glBindVertexArray(0);
}

void ApplicationSolar::initializeTextures() {
    planet current_planet;
    pixel_data planet_data;
    //for(std::vector<planet>::size_type i = 0; i != planets.size(); i++) {
    for(auto& i : planets){
     //current_planet = planets.at(i);
     planet_data = texture_loader::file(m_resource_path + "textures/"+i.name+"map.png");

     //std::cout << current_planet.name << " " << i << m_resource_path + "textures/"+current_planet.name+"map.png" << std::endl;
  
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &i.p_texture.handle);
    glBindTexture(GL_TEXTURE_2D, i.p_texture.handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,planet_data.channels,planet_data.width,planet_data.height,
                0,planet_data.channels,planet_data.channel_type,planet_data.ptr());
  }
}

void ApplicationSolar::initializeFrameBuffer() {
  glGenRenderbuffers(1, &rb_texture.handle);
  glBindRenderbuffer(GL_RENDERBUFFER, rb_texture.handle);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 640u, 480u);


    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &t_texture.handle);
    glBindTexture(GL_TEXTURE_2D, t_texture.handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,640u,480u,
                0,GL_RGB,GL_UNSIGNED_BYTE,NULL);

    glGenFramebuffers(1,&fb_texture.handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_texture.handle);
    glFramebufferTexture(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0, t_texture.handle, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER, rb_texture.handle);

    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE){
      std::cout << "Framebuffer not complete" << std::endl;
    }
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteBuffers(1, &star_object.element_BO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);
  glDeleteBuffers(1, &orbit_object.vertex_BO);
  glDeleteBuffers(1, &orbit_object.element_BO);
  glDeleteVertexArrays(1, &orbit_object.vertex_AO);
}

// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<ApplicationSolar>(argc, argv);
}