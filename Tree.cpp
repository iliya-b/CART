#include <iostream>

#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <sstream>
#include <string>
#include <fstream>
#include <set>

#include "Node.cpp"
#include "Matrix.cpp"

struct Split
{
    int feature;
    double threshold;
    std::vector<int> left;
    std::vector<int> right;
};

class Tree
{

    Matrix<int> data;
    Node *root;

    std::vector<std::set<int>> column_values;

public:
    std::set<int> classes;

    Tree(Matrix<int> data) : data(data), column_values(data.column_values()), classes(column_values.back())
    {
        std::vector<int> root_elements(data.rows);

        std::generate(root_elements.begin(), root_elements.end(), [n = 0]() mutable
                      { return n++; });
        root = new Node(root_elements, 0, 0);
    }

    void build(int max_depth)
    {
        this->split(root, max_depth, 1, 0);
    };

    int predict(std::vector<int> row)
    {
        return predict(row, root);
    }

    int predict(std::vector<int> row, Node *node)
    {
        if (!node->left || !node->right)
        {
            return node->result_class;
        }
        if (row.at(node->feature) < node->threshold)
        {
            return predict(row, node->left);
        }
        else
        {
            return predict(row, node->right);
        }
    }

    auto test_split(int feature, double value, std::vector<int> elements)
    {
        std::vector<int> left(0);
        std::vector<int> right(0);
        for (auto row : elements)
        {
            if (data(row, feature) < value)
            {
                left.push_back(row);
            }
            else
            {
                right.push_back(row);
            }
        }
        return std::make_tuple(left, right);
    };

    double get_gini(std::vector<int> elements, int total_instances)
    {
        int size = elements.size();
        if (size == 0)
            return 0;

        double score = 0.;
        for (int class_id : classes)
        {
            int instances_count = 0;
            for (int row : elements)
            {
                instances_count += data(row, -1) == class_id ? 1 : 0;
            }
            double p = double(instances_count) / size;
            score += p * p;
        }
        return (1. - score) * (double(size) / total_instances);
    };

    struct Split get_split(Node *node)
    {
        int b_index = 999;
        double b_value = 999.;
        double b_score = 1.1;
        std::vector<int> b_left;
        std::vector<int> b_right;

        for (int column = 0; column < data.cols - 1; column++)
        {
            int previous_value;
            int step = 0;
            for (int value : column_values.at(column))
            {
                double split_value = step == 0 ? double(value) : double(previous_value + value) / 2.;
                auto split = test_split(column, split_value, node->elements);
                auto left = std::get<0>(split);
                auto right = std::get<1>(split);

                int total_count = left.size() + right.size();
                double gini = get_gini(left, total_count) + get_gini(right, total_count);
                // std::cout << "X" << (column + 1) << " < " << split_value << " Gini=" << gini << " size=" << total_count << std::endl;
                if (gini < b_score)
                {
                    b_index = column;
                    b_value = split_value;
                    b_score = gini;
                    b_left = left;
                    b_right = right;
                }
                previous_value = value;
                step++;
            }
        }
        struct Split res = {b_index, b_value, b_left, b_right};
        return res;
    };

    void split(Node *node, std::vector<int> left, std::vector<int> right, int max_depth, int min_size, int depth)
    {
        if (!left.size() || !right.size())
        {
            terminate(node);
            return;
        }
        node->left = new Node(left, 0, 0, depth + 1);
        node->right = new Node(right, 0, 0, depth + 1);

        if (max_depth && depth > max_depth)
        {
            terminate(node->left);
            terminate(node->right);
        }
        else
        {

            if (left.size() < min_size)
            {
                terminate(node->left);
            }
            else
            {
                split(node->left, max_depth, min_size, depth + 1);
            }

            if (right.size() < min_size)
            {
                terminate(node->right);
            }
            else
            {
                split(node->right, max_depth, min_size, depth + 1);
            }
        }
        node->leafs_count = node->right->leafs_count + node->left->leafs_count;
    };

    void split(Node *node, int max_depth, int min_size, int depth)
    {
        auto best_split = get_split(node);
        node->feature = best_split.feature;
        node->threshold = best_split.threshold;
        split(node, best_split.left, best_split.right, max_depth, min_size, depth + 1);
    };

    void terminate(Node *node)
    {
        std::map<int, int> classSet;
        int max_count = 0;

        for (auto row : node->elements)
        {
            int current_class = data(row, -1);
            classSet[current_class] += 1;
            int curr_val = classSet[current_class];
            if (curr_val > max_count)
            {
                max_count = curr_val;
                node->result_class = current_class;
            }
        }
    };
};
