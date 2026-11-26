#ifndef POINTS_H
#define POINTS_H

#include <vector>

// A single object's location in the unit square.
struct Point
{
  double x;
  double y;
};

using Points = std::vector<Point>;

#endif
