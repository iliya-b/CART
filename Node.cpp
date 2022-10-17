#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <fstream>
#include <set>

class Node
{

    int depth = 0;

public:
    Node *left = NULL;
    Node *right = NULL;

    double threshold = 0;
    int feature = 0;
    std::vector<int> elements;
    int result_class = 0;
    int leafs_count = 1;

    Node(std::vector<int> elements, int feature, double threshold, int depth = 0) : elements(elements), feature(feature), threshold(threshold), depth(depth)
    {
    }
};
