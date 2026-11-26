#include "nearest_furthest.h"

#include <fstream>
#include <limits>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "distance.h"

namespace
{
  // Finds the nearest and furthest keys from points[i] to all other points.
  // Keys are not always equal to the points distance, as in the single thread computation the values remain squared until the /smallest/largest values are found
  template <typename Metric>
  void scanObject(const Points& points, std::size_t i, double& nearestKey, double& furthestKey)
  {
    // Initialises min/maxKey to extremities to ensure real point values are used 
    double minKey = std::numeric_limits<double>::max();
    double maxKey = -1.0;
    const Point& p = points[i];
    const std::size_t n = points.size();

    // Loop to find the closest and furthest points and store them in nearestKey/furthestKey
    for (std::size_t j = 0; j < n; ++j)
    {
      if (j == i) continue; // Prevents points from comparing to themselves
      const double k = Metric::key(p, points[j]); // Calls the key function from distance.h, with metric determining which calculation to perform
      if (k < minKey) minKey = k;
      if (k > maxKey) maxKey = k;
    }
    nearestKey = minKey;
    furthestKey = maxKey;
  }

  template <typename Metric>
  Results serialImpl(const Points& points)
  {
    // Finds number of points and creates a new object 'r' with two arrays assigned to it
    const long n = static_cast<long>(points.size());
    Results r;
    r.nearest.resize(n);
    r.furthest.resize(n);

    for (long i = 0; i < n; ++i)
    {
      double nk, fk; // nearest/furthest key received from scanObject function above
      scanObject<Metric>(points, i, nk, fk);
      // Calls 'toDistance' function from distance.h - metric determines whether it is Standard or Manhattan calculation - then places the value into its respective array
      r.nearest[i] = Metric::toDistance(nk);
      r.furthest[i] = Metric::toDistance(fk);
    }

    double sumNearest = 0.0, sumFurthest = 0.0;
    /// Mean calculations
    for (long i = 0; i < n; ++i)
    {
      sumNearest += r.nearest[i];
      sumFurthest += r.furthest[i];
    }
    r.meanNearest = sumNearest / n;
    r.meanFurthest = sumFurthest / n;
    return r;
  }

  // Similar function to the serialImplementation, but makes use of multiple threads
  template <typename Metric>
  Results parallelImpl(const Points& points)
  {
    const long n = static_cast<long>(points.size());
    Results r;
    r.nearest.resize(n);
    r.furthest.resize(n);
    // Creates new arrays, independent of r to prevent clashes when using multiple threads
    double* nearest = r.nearest.data();
    double* furthest = r.furthest.data();

    #pragma omp parallel for schedule(runtime)
    // Same loops as the single thread implementation, just making use of parallel threads
    for (long i = 0; i < n; ++i)
    {
      double nk, fk;
      scanObject<Metric>(points, i, nk, fk);
      nearest[i] = Metric::toDistance(nk);
      furthest[i] = Metric::toDistance(fk);
    }

    double sumNearest = 0.0, sumFurthest = 0.0;
    #pragma omp parallel for reduction(+ : sumNearest, sumFurthest)
    for (long i = 0; i < n; ++i)
    {
      sumNearest += nearest[i];
      sumFurthest += furthest[i];
    }
    r.meanNearest = sumNearest / n;
    r.meanFurthest = sumFurthest / n;
    return r;
  }

  // Checks that only two points are being compared
  void checkTwoPoints(const Points& points)
  {
    if (points.size() < 2)
      throw std::runtime_error("At least 2 locations are needed to find nearest/furthest points");
  }
}

// Compute functions called from main.cpp to calc distances
// Both check for a valid number of points using checkTwoPoints
Results computeSerial(const Points& points, DistanceType type)
{
  checkTwoPoints(points);
  return type == DistanceType::Standard ? serialImpl<StandardMetric>(points) : serialImpl<ManhattanMetric>(points);
}

Results computeParallel(const Points& points, DistanceType type, int numThreads, Schedule schedule, int chunk)
{
  checkTwoPoints(points);

// Checks whether OpenMP is being used, if so uses numThreads from the user input when executing the nearest_furthest file
// Also checks to see whether a schedule was specified on compilation, if so it sets the omp schedule as necessary
#ifdef _OPENMP
  if (numThreads > 0) omp_set_num_threads(numThreads);
  omp_sched_t kind = omp_sched_static;
  if (schedule == Schedule::Dynamic) kind = omp_sched_dynamic;
  else if (schedule == Schedule::Guided) kind = omp_sched_guided;
  omp_set_schedule(kind, chunk > 0 ? chunk : 0);
#else
  (void)numThreads; (void)schedule; (void)chunk;
#endif

  return type == DistanceType::Standard ? parallelImpl<StandardMetric>(points)
                                        : parallelImpl<ManhattanMetric>(points);
}

// Creates the nearest/furthest csv files with one distance value per line
void writeValues(const std::string& filename, const std::vector<double>& values)
{
  std::ofstream out(filename);
  if (!out) throw std::runtime_error("Cannot open '" + filename + "' for writing"); // Checks the file can be created/written to (Had a problem with the file not being created so kept this as a check)
  for (double v : values) out << v << '\n';
}
