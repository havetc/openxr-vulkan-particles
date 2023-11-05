#pragma once

#include <vector>
#include <future>
#include <glm/glm.hpp>

struct Particle
{
  glm::vec3 position;
  glm::vec3 speed;
  float mass;
  Particle* fusion = nullptr;
};

class Particles
{
public:
  Particles(int particle_number=2000);

  void cpu_update_nbody();

  void async_update_nbody();
  bool result_ready()
  {
    return !computing;
  }

  size_t getSize() const;
  unsigned int getParticleNumber() const
  {
    return particles.size();
  }

  void copyTo(char* destination) const;

  ~Particles();

protected:
  std::vector<struct Particle> particles;
  std::atomic_bool computing = false;
  std::future<void> res;
};