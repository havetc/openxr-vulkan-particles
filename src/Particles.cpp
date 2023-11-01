#include "Particles.h"
#include <random>
#include <algorithm>
#include <execution>

float TIMESTEP = 0.001f;
float GRAVITY_CONST = 0.0001f;

Particles::Particles(int particle_number) : particles(particle_number)
{
  std::default_random_engine rndEngine((unsigned)time(nullptr));
  std::uniform_real_distribution<float> rndDist(-2.0f, 2.0f);
  std::uniform_real_distribution<float> rndMass(10.0f, 100.0f);

  // Initial particle positions
  for (auto& particle : particles)
  {
    particle.position = glm::vec3(rndDist(rndEngine), rndDist(rndEngine)+2, rndDist(rndEngine));
    particle.speed = glm::vec3(0.0f);
    particle.mass = rndMass(rndEngine);
  }
}

//TODO: in case of nan, maybe check for p1 position != p2
inline glm::vec3 gravity_force_on_p1(const Particle & p1, const Particle & p2)
{
  return glm::normalize(p2.position - p1.position) * p1.mass * p2.mass * GRAVITY_CONST / powf(glm::distance(p1.position, p2.position), 2);
}

//Naive brute force implementation
void Particles::cpu_update_nbody()
{
  computing = true;
  // to avoid updates on some particles changing the gravity computation of the next one at the
  // same timestep, we first update all particles speeds (gravity depends only on position, so no
  // influence on computation), and then afterward we update all positions
  std::for_each(std::execution::par_unseq, particles.begin(), particles.end(),
                [this](Particle& p1)
                {
                  glm::vec3 gravity(0, 0, 0);
                  for (const auto& p2 : particles)
                  {
                    if (&p1 != &p2) // infinite gravity between a particle and itself, so we must avoid that
                    {
                      gravity += gravity_force_on_p1(p1, p2);
                    }
                  }
                  p1.speed += gravity / p1.mass * TIMESTEP; // acceleration = Force / Mass -> in our case Gravity / Mass
                });

  // update positions
  for (auto& p : particles)
  {
    p.position += p.speed * TIMESTEP;
  }
  computing = false;
}

void Particles::async_update_nbody()
{
  res = std::async(std::launch::async, &Particles::cpu_update_nbody, this);
}

size_t Particles::getSize() const
{
  return sizeof(Particle)*particles.size();
}

void Particles::copyTo(char* destination) const
{
  memcpy(destination, particles.data(), getSize()); // Vertex section first
}

Particles::~Particles()
{
  particles.clear();
}

