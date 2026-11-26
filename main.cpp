/*
 Usage (Cannot use n and f or s and p, must pick one of each):
   -n <count>     generate <count> random locations
   -f <filename>  read locations from a CSV file
   -d std|man     standard (L2) or Manhattan (L1) distance
   -s             run serial code
   -p             run parallel code
   (Given in the exercise requirements)
 Extras:
   -t <threads>   number of threads
   -k <schedule>  static | dynamic | guided
   -c <chunk>     loop schedule chunk size 
   -r <seed>      seed for random location generation - allows for consistent results
*/

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "csv_reader.h"
#include "nearest_furthest.h"
#include "points.h"
#include "rng.h"

namespace
{
  // Initialises all of the variables influenced by the user input
  struct Options
  {
    long numLocations = 0;
    std::string filename;
    DistanceType distance = DistanceType::Standard;
    bool haveDistance = false;
    bool serial = false;
    bool parallel = false;
    int threads = 0;
    Schedule schedule = Schedule::Static;
    int chunk = 0;
    bool haveSeed = false;
    unsigned seed = 0;
  };

  void printUsage(const char* prog)
  {
    // Output to terminal if the executable is ran incorrectly
    std::cerr << "Usage: " << prog << " (-n <random_count> | -f <file>) -d (std|man) (-s | -p)\n"
              << "[-t <num_of_threads>] [-k static|dynamic|guided] [-c <chunk_num>] [-r <seed>]\n";
  }

  Options parseArgs(int arg_count, char* arg_vect[])
  {
    Options opt;
    // Used to return the value after an argument from the running of the executable
    auto needValue = [&](int& i) -> std::string
    {
      // Checks if there is value after the 'current' argument, if not throw an error message.
      if (i + 1 >= arg_count) throw std::runtime_error(std::string("Missing value after ") + arg_vect[i]);
      // Returns the next argument
      return arg_vect[++i];
    };

    // Runs through all argument given and assigns values as required
    // Starts at i = 1 as i = 0 is the executable name
    for (int i = 1; i < arg_count; ++i)
    {
      const std::string flag = arg_vect[i];
      if (flag == "-n")
      {
        opt.numLocations = std::stol(needValue(i));
        if (opt.numLocations < 2) throw std::runtime_error("-n must be at least 2");
      }
      else if (flag == "-f") opt.filename = needValue(i);
      else if (flag == "-d")
      {
        const std::string v = needValue(i);
        if (v == "std") opt.distance = DistanceType::Standard;
        else if (v == "man") opt.distance = DistanceType::Manhattan;
        else throw std::runtime_error("-d must be 'std' or 'man'");
        opt.haveDistance = true;
      }
      else if (flag == "-s") opt.serial = true;
      else if (flag == "-p") opt.parallel = true;
      else if (flag == "-t") opt.threads = std::stoi(needValue(i));
      else if (flag == "-k")
      {
        const std::string v = needValue(i);
        if (v == "static") opt.schedule = Schedule::Static;
        else if (v == "dynamic") opt.schedule = Schedule::Dynamic;
        else if (v == "guided") opt.schedule = Schedule::Guided;
        else throw std::runtime_error("-k must be 'static', 'dynamic' or 'guided'");
      }
      else if (flag == "-c") opt.chunk = std::stoi(needValue(i));
      else if (flag == "-r")
      {
        opt.seed = static_cast<unsigned>(std::stoul(needValue(i)));
        opt.haveSeed = true;
      }
      else throw std::runtime_error("Unknown flag '" + flag + "'");
    }

    // Checks there are no overlapping flags
    const bool have_num = opt.numLocations > 0;
    const bool have_file = !opt.filename.empty();
    if (have_num == have_file) throw std::runtime_error("Specify exactly one of -n and -f");
    if (opt.serial == opt.parallel) throw std::runtime_error("Specify exactly one of -s and -p");
    if (!opt.haveDistance) throw std::runtime_error("Specify the distance type with -d");
    return opt;
  }

  Points generateRandomPoints(long n)
  {
    Points points;
    points.reserve(n);
    for (long i = 0; i < n; ++i)
    {
      const double x = utils::rand::randDouble(0.0, 1.0);
      const double y = utils::rand::randDouble(0.0, 1.0);
      points.push_back({x, y});
    }
    return points;
  }
}

int main(int arg_count, char* arg_vect[])
{
  try
  {
    const Options opt = parseArgs(arg_count, arg_vect);

    if (opt.haveSeed) utils::rand::seedRand(opt.seed);
    const Points points = opt.filename.empty() ? generateRandomPoints(opt.numLocations)
                                               : utils::csv::readPoints(opt.filename);

    // Time only the calculation (not file reading or output).
    const auto start = std::chrono::steady_clock::now();
    const Results results = opt.parallel
        ? computeParallel(points, opt.distance, opt.threads, opt.schedule, opt.chunk)
        : computeSerial(points, opt.distance);
    const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;

    writeValues("nearest.csv", results.nearest);
    writeValues("furthest.csv", results.furthest);

    std::cout << "Mean distance to nearest neighbour: " << results.meanNearest << '\n'
              << "Mean distance to furthest neighbour: " << results.meanFurthest << '\n'
              << "Time taken: " << elapsed.count() << " seconds" << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << '\n';
    printUsage(arg_vect[0]);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
