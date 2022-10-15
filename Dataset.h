#include <vector>

class Dataset
{
public:
    int cols;
    int rows;
    int total_classes;

    Dataset(int rows, int cols, std::vector<double> vector, int total_classes);

    double operator()(int x, int y);

protected:
    std::vector<double> vector;
};