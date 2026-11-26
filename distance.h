#ifndef DISTANCE_H
#define DISTANCE_H

#include <cmath>
#include "points.h"

class StandardMetric
{
public:
  // Compares the distance from point a to b and returns dx^2 + dy^2
  static double key(const Point& a, const Point& b)
  {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return dx * dx + dy * dy;
  }
  // Separate function to return the actual distance from point to point by square rooting the value returned from key
  // Separate to reduce processing, as this is only needed once the nearest and furthest points have been identified
  static double toDistance(double k) 
  {
     return std::sqrt(k);
  }
};

class ManhattanMetric
{
public:
  // Returns the Manhattan distance between points
  static double key(const Point& a, const Point& b)
  {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
  }
  // Even though this function is unecessary here it made the code in nearest_furthest.cpp simpler as the program can run independent of the metric type.
  static double toDistance(double k)
  {
    return k;
  }
};

#endif
