#ifndef CSV_READER_H
#define CSV_READER_H

#include <string>
#include <vector>
#include "points.h"

namespace utils
{
  namespace csv
  {
    std::vector<std::vector<double>> readCSV(const std::string& filename);

    // Reads a CSV file where each line is "x,y" into an array of Points.
    Points readPoints(const std::string& filename);
  }
}

#endif
