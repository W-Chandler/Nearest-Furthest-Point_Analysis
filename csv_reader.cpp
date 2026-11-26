#include "csv_reader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace utils
{
  namespace csv
  {
    namespace
    {
      // Removes leading/trailing whitespace
      std::string trim(const std::string& s)
      {
        const char* ws = " \t\r\n"; // Specific whitespace values to be removed
        const auto first = s.find_first_not_of(ws);
        if (first == std::string::npos) return ""; // Returns blank for empty spaces
        const auto last = s.find_last_not_of(ws); 
        return s.substr(first, last - first + 1); //Returns the trimmed value as a string
      }
      
      // Converts strings from the csv file to doubles
      double parseDouble(const std::string& field, std::size_t lineNo)
      {
        // Checks that no values with characters other than valid numbers are accepted
        try
        {
          std::size_t used = 0;
          const double value = std::stod(field, &used);
          if (used != field.size()) throw std::invalid_argument(field);
          return value;
        }
        // If any issues with the csv file get to this point, a clear description is given
        catch (const std::exception&)
        {
          throw std::runtime_error("CSV line " + std::to_string(lineNo) +
                                   ": cannot interpret '" + field + "' as a number");
        }
      }
    }

    std::vector<std::vector<double>> readCSV(const std::string& filename)
    {
      // opens file for reading, reports error if it cannot be opened
      std::ifstream in(filename);
      if (!in) throw std::runtime_error("Cannot open file '" + filename + "'");


      std::vector<std::vector<double>> rows;
      std::string line;
      std::size_t lineNo = 0;

      // Reads all lines of the file
      while (std::getline(in, line))
      {
        ++lineNo;
        line = trim(line);
        if (line.empty()) continue;

        std::vector<double> row;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, ','))
        {
          // Converts the values to a double (lineNo used for error catching)
          row.push_back(parseDouble(trim(field), lineNo));
        }
        // Adds the completed row into rows
        rows.push_back(std::move(row));
      }
      return rows;
    }

    Points readPoints(const std::string& filename)
    {
      const auto rows = readCSV(filename);

      Points points;
      points.reserve(rows.size()); // Reserves space for the number of points
      for (std::size_t i = 0; i < rows.size(); ++i)
      {
        // Checks that rows are exactly 2 values long
        if (rows[i].size() != 2)
        {
          throw std::runtime_error("Row " + std::to_string(i + 1) + " of '" + filename +
                                   "' should contain exactly 2 values (x,y)");
        }
        // Adds a point
        points.push_back({rows[i][0], rows[i][1]});
      }
      return points;
    }
  }
}
