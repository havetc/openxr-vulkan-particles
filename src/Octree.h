#pragma once
#include <vector>

uint32_t morton_code(float range, float x, float y, float z);

bool is_neighbour(uint32_t morton_code1, uint32_t morton_code2, unsigned int depth);

/// <summary>
/// The basic element of an octree: contains the mean of contained objects, and contains either objects (leaf) or a subdivision in 8 subnode (2*2*2)
/// Objects are of class T
/// </summary>
template <class T> class Node{
public:
  Node<T>(uint32_t depth, uint32_t morton_code) : depth(depth), morton_coord(morton_code)
  {
    static_assert(depth >= 0);
  };

  const std::vector<T>& get_elems(uint32_t morton_code) const
  {
    if (depth == 0 || sub_nodes == nullptr)
    {
      return elems;
    }
    else {
      return sub_nodes[(morton_coord & (0b111 << (3*depth))) >> (3*depth) ].get_elems(morton_code);
    }
  }

  void put_elem(const T & elem, uint32_t morton_code)
  {
    if (mean != nullptr)
    {
      *mean += elem;
    }
    else
    {
      mean = new T(elem);
    }

    if (depth == 0)
    {
      elems.push_back(elem);
    }
    else
    {
      if (sub_nodes == nullptr)
      {
        subdivide();
      }
      return sub_nodes[(morton_coord & (0b111 << (3 * depth))) >> (3 * depth)].put_elem(elem, morton_code);
    }
  }

  void get_leaves(std::vector<Node<T> *> & leaves)
  {
    if (depth == 0)
    {
      if (!is_empty())
      {
        leaves.push_back(this);
      }
    }
    else if (sub_nodes != nullptr)
    {
      for (Node<T>& node : sub_nodes)
      {
        node.get_leaves(leaves);
      }
    }
  }

  /// <summary>
  /// We don't look the 26 neighbors: only the 7 positives (x >= , y >= and z >= ) which is more efficient
  /// to compute with morton codes.
  /// However, that means that each interaction between two different nodes will be accounted only once:
  /// the physics engine must take that into account, as when a particle is matched with another one, the force
  /// is computed and then BOTH particles have their characteristics updated
  /// within a same node, we can do like before
  /// </summary>
  /// <param name="leaves"></param>
  void get_positives_neighbors(std::vector<Node<T>*>& nodes, uint32_t morton_code)
  {
    if (depth == 0 && ! is_empty())
    {
      nodes.push_back(this);
    }
    else
    {

    }
  }

  void subdivide()
  {
    sub_nodes = { Node<T>(depth - 1, morton_code | (0b000 << (3*depth))), Node<T>(depth - 1, morton_code | (0b001 << (3*depth))),
                   Node<T>(depth - 1, morton_code | (0b010 << (3*depth))), Node<T>(depth - 1, morton_code | (0b011 << (3*depth))),
                   Node<T>(depth - 1, morton_code | (0b100 << (3*depth))), Node<T>(depth - 1, morton_code | (0b101 << (3*depth))),
                   Node<T>(depth - 1, morton_code | (0b110 << (3*depth))), Node<T>(depth - 1, morton_code | (0b111 << (3*depth))) };
  }

  bool is_empty() const
  {
    return elems.size() == 0;
  }

  ~Node<T>()
  {
    delete[] sub_nodes;
    delete mean;
  };

protected:
  // store node coordinate as a morton code, with depth its not ambiguous. Could allow node storage/access in a hash table.
  // Also, Node index in subnodes can be retrieved from a mask in input given -> ( & 0b111 << 10 - depth) >> 10 - depth
  uint32_t morton_coord;
  uint32_t depth;
  // not really the "mean" of contained elements: more like the sum of them contained in one point.
  T* mean = nullptr;
  Node<T> sub_nodes[8] = nullptr;
  std::vector<T> elems;
};

template <class T> class Octree
{
public:
  Octree<T>(float max_size = 100.0, unsigned int max_depth = 10) : max_size(max_size), max_depth(max_depth)
  {
    root_nodes = { Node<T>(max_depth - 1, 0b000 << 27), Node<T>(max_depth - 1, 0b001 << 27),
                   Node<T>(max_depth - 1, 0b010 << 27), Node<T>(max_depth - 1, 0b011 << 27),
                   Node<T>(max_depth - 1, 0b100 << 27), Node<T>(max_depth - 1, 0b101 << 27),
                   Node<T>(max_depth - 1, 0b110 << 27), Node<T>(max_depth - 1, 0b111 << 27) };
  };

  void put_elem(const T& elem, uint32_t morton_code)
  {
    return root_nodes[(morton_code & (0b111 << 27)) >> 27].put_elem(elem, morton_code);
  }

  /// <summary>
  /// return a list of pointer towards every leaf node (by definition at max depth) which are not empty
  /// It aims to be combined with octree iteration to get neighbors and approximations, because as every
  /// elements within a same node has the exact same neighbors / approximing Nodes, it would be wasteful
  /// to iterate over elements and not nodes.
  /// </summary>
  /// <returns> a std::vector<Node<T>*>, which is a list of pointer towards non empty nodes</returns>
  std::vector<Node<T> *> get_leaves()
  {
    std::vector<Node<T>*> res;
    for (Node<T>& node : root_nodes)
    {
      node.get_leaves(res);
    }
    return res;
  }

  ~Octree<T>()
  {
    delete[] root_nodes;
  };

protected:
  float max_size; // octree provides storage in -max_size to max_size in each dimension
  unsigned int max_depth;
  Node<T> root_nodes[8] = nullptr;
};