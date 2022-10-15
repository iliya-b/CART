#include <iostream>

#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <sstream>
#include <string>
#include <fstream>

template <class T>
class Matrix
{
public:
  int cols;
  int rows;
  int total_classes;

  Matrix(int rows, int cols, std::vector<T> vector, int total_classes) : rows(rows), cols(cols), vector(vector), total_classes(total_classes){};

  T operator()(int x, int y)
  {
    return vector[cols * x + y];
  }

protected:
  std::vector<T> vector;
};

struct Split
{
  int feature;
  double threshold;
  std::vector<int> left;
  std::vector<int> right;
};

template <class R>
class Node
{

  int depth = 0;

public:
  Node *left = NULL;
  Node *right = NULL;

  R threshold = 0;
  int feature = 0;
  std::vector<int> elements;
  int result_class = 0;

  Node(std::vector<int> elements, int feature, R threshold, int depth = 0) : elements(elements), feature(feature), threshold(threshold), depth(depth)
  {
  }
};

template <class T>
class Tree
{

  Matrix<T> data;
  Node<T> *root;

public:
  Tree(Matrix<double> data) : data(data)
  {
    std::vector<int> root_elements(data.rows);

    std::generate(root_elements.begin(), root_elements.end(), [n = 0]() mutable
                  { return n++; });
    root = new Node<double>(root_elements, 0, 0);
  }

  void build_tree()
  {
    this->split(root, 1000, 1);
    // std::cout << s.threshold << std::endl;
    // std::cout << root->threshold << std::endl;
  };

  int predict(std::vector<double> row)
  {
    return predict(row, root);
  }
  int predict(std::vector<double> row, Node<T> *node)
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
    for (int class_id = 0; class_id < data.total_classes; class_id++)
    {
      int instances_count = 0;
      for (int row : elements)
      {
        instances_count += data(row, -1) == class_id ? 1 : 0;
      }
      double p = instances_count / size;
      score += p * p;
    }
    return (1. - score) * (size / total_instances);
  };

  struct Split get_split(Node<T> *node)
  {
    int b_index = 999;
    T b_value = 999.;
    double b_score = 1.1;
    std::vector<int> b_left(0);
    std::vector<int> b_right(0);

    for (int column = 0; column < data.cols - 1; column++)
    {
      for (int row = 0; row < data.rows; row++)
      {
        T value = data(row, column);
        auto split = test_split(column, value, node->elements);
        auto left = std::get<0>(split);
        auto right = std::get<1>(split);

        int total_count = left.size() + right.size();
        double gini = get_gini(left, total_count) + get_gini(right, total_count);
        // std::cout << "X" << (column + 1) << " < " << value << " Gini=" << gini << std::endl;
        if (gini < b_score)
        {
          b_index = column;
          b_value = value;
          b_score = gini;
          b_left = left;
          b_right = right;
        }
      }
    }
    struct Split res = {b_index, b_value, b_left, b_right};
    return res;
  };

  void split(Node<T> *node, std::vector<int> left, std::vector<int> right, int max_depth, int min_size, int depth)
  {
    if (!left.size() || !right.size())
    {
      terminate(node);
      return;
    }
    node->left = new Node<double>(left, 0, 0, depth + 1);
    node->right = new Node<double>(right, 0, 0, depth + 1);

    if (depth > max_depth)
    {
      terminate(node->left);
      terminate(node->right);
      return;
    }

    if (left.size() < min_size)
    {
      terminate(node->left);
    }
    else
    {
      split(node->left, max_depth, min_size);
    }

    if (right.size() < min_size)
    {
      terminate(node->right);
    }
    else
    {
      split(node->right, max_depth, min_size);
    }
  };

  void split(Node<T> *node, int max_depth, int min_size)
  {
    auto best_split = get_split(node);
    node->feature = best_split.feature;
    node->threshold = best_split.threshold;

    split(node, best_split.left, best_split.right, max_depth, min_size, 0);
  };

  void terminate(Node<T> *node)
  {
    std::map<T, int> classSet;
    int max_count = 0;

    for (auto row : node->elements)
    {
      double current_class = data(row, -1);
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

std::vector<std::vector<double>> parseCSV(std::string filename)
{
  std::ifstream data(filename);
  std::string line;
  std::vector<std::vector<double>> parsedCsv;
  while (std::getline(data, line))
  {
    std::stringstream lineStream(line);
    std::string cell;
    std::vector<double> parsedRow;
    while (std::getline(lineStream, cell, ','))
    {
      parsedRow.push_back(atof(cell.c_str()));
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

Matrix<double> make_dataset1()
{

  std::vector<double> raw_dataset{
      2.771244718, 1.784783929, 0,
      1.728571309, 1.169761413, 0,
      3.678319846, 2.81281357, 0,
      3.961043357, 2.61995032, 0,
      2.999208922, 2.209014212, 0,
      7.497545867, 3.162953546, 1,
      9.00220326, 3.339047188, 1,
      7.444542326, 0.476683375, 1,
      10.12493903, 3.234550982, 1,
      6.642287351, 3.319983761, 1};

  return Matrix<double>(10, 3, raw_dataset, 2);
}

auto dataset2_train()
{

  auto lines = parseCSV("train.txt");
  return Matrix<double>(lines.size(), lines[0].size(), flatten(lines), 2);
}

auto dataset2_test()
{
  return parseCSV("test.txt");
}

void confusion_matrix()
{
}

int main()
{

  auto dataset = dataset2_train();
  Tree<double> tree(dataset);
  tree.build_tree();

  std::vector<int> mat(4);

  auto test_dataset = dataset2_test();
  for (auto line : test_dataset)
  {
    auto predict = tree.predict(line);
    auto actual = line[line.size() - 1];
    mat[predict + dataset.total_classes * actual] += 1;

    std::cout
        << "PREDICT: " << predict << " ACTUAL: " << actual << std::endl;
  }

  double acc = 0;
  for (int i = 0; i < dataset.total_classes; i++)
  {
    int sum = 0;
    for (int j = 0; j < dataset.total_classes; j++)
    {
      sum += mat[i + dataset.total_classes * j];
    }
    acc += double(mat[i + dataset.total_classes * i]) / sum;
  }
  std::cout << "ACC: " << (acc / dataset.total_classes) << std::endl;

  return 0;
}
