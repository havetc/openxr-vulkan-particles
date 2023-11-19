#include "Octree.h"
#include <algorithm>

uint32_t expandBits(uint32_t v)
{
  // v = 0000 0000 0000 0000 0000 00AA BBCC DDEE
  //  a mult 0000 0000 0000 0001 0000 0000 0000 0001   =>  a | a << 16 (a < 1024 so a is only 10 bits max)
  // mask 1111 1111 0000 0000 0000 0000 1111 1111 => 0000 00AA 0000 0000 0000 0000 BBCC DDEE
  v = (v * 0x00010001u) & 0xFF0000FFu;

  // => 0000 00AA 0000 0000 BBCC 0000 0000 DDEE
  v = (v * 0x00000101u) & 0x0F00F00Fu;
  // => 0000 00AA 0000 BB00 00CC 0000 DD00 00EE
  v = (v * 0x00000011u) & 0xC30C30C3u;
  // => 0000 A00A 00B0 0B00 C00C 00D0 0D00 E00E
  v = (v * 0x00000005u) & 0x49249249u;
  return v;
}


uint32_t morton_code(float range, float x, float y, float z)
{
  uint32_t res = 0;
  //normalization of input in 0 <-> 1.0f range
  x = (x + range) / (2 * range);
  y = (y + range) / (2 * range);
  z = (z + range) / (2 * range);
  
  //https://developer.nvidia.com/blog/thinking-parallel-part-iii-tree-construction-gpu/
  // normalization of input in 0 <-> 1023.0f range
  x = std::min(std::max(x * 1024.0f, 0.0f), 1023.0f);
  y = std::min(std::max(y * 1024.0f, 0.0f), 1023.0f);
  z = std::min(std::max(z * 1024.0f, 0.0f), 1023.0f);
  
  uint32_t xx = expandBits((uint32_t)x);
  uint32_t yy = expandBits((uint32_t)y);
  uint32_t zz = expandBits((uint32_t)z);
  
  return (xx << 2) + (yy << 1) + zz;

  return 0;
}

bool is_neighbour(uint32_t morton_code1, uint32_t morton_code2, unsigned int depth)
{
  uint32_t t = morton_code1 ^ morton_code2;
  
  //maybe LUT for 3*(depth+1) to optimize a little ? depth has only 10 possibility between 0 and 9
  return ((t << (3 * (depth + 1))) & t) == ((0xFFFFFFFF << (3 * (depth + 1))) & t);
}
