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
  size_t getSize() const;
  unsigned int getParticleNumber() const
  {
    return particles.size();
  }

  void copyTo(char* destination) const;

  ~Particles();

protected:
  std::vector<struct Particle> particles;
};