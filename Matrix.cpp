#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <fstream>
#include <set>

template <class T>
class Matrix
{
public:
    int cols;
    int rows;

    Matrix(int rows, int cols, std::vector<T> vector) : rows(rows), cols(cols), vector(vector){};

    std::vector<std::set<T>> column_values()
    {
        std::vector<std::set<T>> columns(cols);
        for (int i = 0; i < cols; i++)
        {
            std::set<T> col_values;
            for (int j = 0; j < rows; j++)
            {
                col_values.insert(vector.at(cols * j + i));
            }
            columns[i] = col_values;
        }
        return columns;
    }

    T operator()(int x, int y)
    {
        return vector[cols * x + y];
    }

protected:
    std::vector<T> vector;
};
