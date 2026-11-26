#ifndef NEAREST_FURTHEST_H
#define NEAREST_FURTHEST_H

#include <string>
#include <vector>
#include "points.h"

enum class DistanceType {Standard, Manhattan};

enum class Schedule {Static, Dynamic, Guided};

struct Results
{
  // Initialises nearest/furthest arrays to store the distance of the nearest/furthest points from each point 'i'
  std::vector<double> nearest;
  std::vector<double> furthest;
  double meanNearest = 0.0;
  double meanFurthest = 0.0;
};

// Function to calculate distances on a single thread. Takes the type of distance to calculate from the users input on running the nearest_furthest executable
Results computeSerial(const Points& points, DistanceType type);

// Calculates distances using threads in parallel
Results computeParallel(const Points& points, DistanceType type, int numThreads = 0, Schedule schedule = Schedule::Static, int chunk = 0);

// Writes one value per line, default formatting/precision.
void writeValues(const std::string& filename, const std::vector<double>& values);

#endif
