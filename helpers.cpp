#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>
#include <fstream>

std::vector<std::vector<int>> parseCSV(std::string filename)
{
    std::ifstream data(filename);
    std::string line;
    std::vector<std::vector<int>> parsedCsv;
    while (std::getline(data, line))
    {
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<int> parsedRow;
        while (std::getline(lineStream, cell, ';'))
        {
            parsedRow.push_back((atoi(cell.c_str())));
        }
        parsedCsv.push_back(parsedRow);
    }
    return parsedCsv;
};

template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T>> &orig)
{
    std::vector<T> ret;
    for (const auto &v : orig)
        ret.insert(ret.end(), v.begin(), v.end());
    return ret;
}
