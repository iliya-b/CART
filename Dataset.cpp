#include "Dataset.h"
#include <vector>

double Dataset::operator()(int x, int y)
{
    return this->vector[cols * x + y];
};

Dataset::Dataset(int rows, int cols, std::vector<double> vector, int total_classes) : rows(rows), cols(cols), vector(vector), total_classes(total_classes){};
