#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Particle
{
  glm::vec3 position;
  glm::vec3 speed;
  float mass;
};

class Particles
{
public:
  Particles(int particle_number=1000);

  void cpu_update_nbody();

  ~Particles();

protected:
  std::vector<Particle> particles;
};