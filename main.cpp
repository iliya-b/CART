#include <iostream>

#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <sstream>
#include <string>
#include <fstream>
#include <set>

template <class T>
class Matrix
{
public:
  int cols;
  int rows;
  int total_classes;

  Matrix(int rows, int cols, std::vector<T> vector, int total_classes) : rows(rows), cols(cols), vector(vector), total_classes(total_classes){};

  std::vector<std::set<T>> column_values()
  {
    std::vector<std::set<T>> columns(cols);
    for (int i = 0; i < cols; i++)
    {
      std::set<T> col_values;
      for (int j = 0; j < rows; j++)
      {
        col_values.insert(vector[cols * j + i]);
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

  double threshold = 0;
  int feature = 0;
  std::vector<int> elements;
  int result_class = 0;

  Node(std::vector<int> elements, int feature, double threshold, int depth = 0) : elements(elements), feature(feature), threshold(threshold), depth(depth)
  {
  }
};

class Tree
{

  Matrix<int> data;
  Node<int> *root;

  std::vector<std::set<int>> column_values;

public:
  std::set<int> classes;

  Tree(Matrix<int> data) : data(data), column_values(data.column_values()), classes(column_values.back())
  {
    std::vector<int> root_elements(data.rows);

    std::generate(root_elements.begin(), root_elements.end(), [n = 0]() mutable
                  { return n++; });
    root = new Node<int>(root_elements, 0, 0);
  }

  void build_tree()
  {
    this->split(root, 1000000, 1);
  };

  int predict(std::vector<int> row)
  {
    return predict(row, root);
  }
  int predict(std::vector<int> row, Node<int> *node)
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
    for (double class_id : classes)
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

  struct Split get_split(Node<int> *node)
  {
    int b_index = 999;
    double b_value = 999.;
    double b_score = 1.1;
    std::vector<int> b_left(0);
    std::vector<int> b_right(0);

    for (int column = 0; column < data.cols - 1; column++)
    {
      int previous_value;
      int step = 0;
      for (int value : column_values[column])
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

  void split(Node<int> *node, std::vector<int> left, std::vector<int> right, int max_depth, int min_size, int depth)
  {
    if (!left.size() || !right.size())
    {
      terminate(node);
      return;
    }
    node->left = new Node<int>(left, 0, 0, depth + 1);
    node->right = new Node<int>(right, 0, 0, depth + 1);

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

  void split(Node<int> *node, int max_depth, int min_size)
  {
    auto best_split = get_split(node);
    // std::cout << (best_split.feature + 1) << " < " << best_split.threshold << std::endl;
    node->feature = best_split.feature;
    node->threshold = best_split.threshold;

    split(node, best_split.left, best_split.right, max_depth, min_size, 0);
  };

  void terminate(Node<int> *node)
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
      parsedRow.push_back(int(atof(cell.c_str()) * 1000));
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

auto dataset2_train()
{

  auto lines = parseCSV("train.txt");
  return Matrix<int>(lines.size(), lines[0].size(), flatten(lines), 3);
}

auto dataset2_test()
{
  return parseCSV("test.txt");
}

void baccuracy()
{
}

int main()
{

  auto dataset = dataset2_train();
  Tree tree(dataset);
  tree.build_tree();

  std::vector<int> mat(4);

  auto test_dataset = dataset2_test();
  int correct_count = 0;
  std::map<std::pair<int, int>, int> confusion;

  for (auto line : test_dataset)
  {
    int predict = tree.predict(line);
    int actual = line[line.size() - 1];
    auto key = std::make_pair(predict, actual);
    confusion[key] = confusion.count(key) ? confusion[key] + 1 : 1;
    correct_count += predict == actual ? 1 : 0;
  }
  std::cout << "basic acc: " << (double(correct_count) / test_dataset.size()) << std::endl;
  double acc = 0;
  for (int class_i : tree.classes)
  {
    int sum = 0;
    for (int class_j : tree.classes)
    {
      sum += confusion[std::make_pair(class_i, class_j)];
    }
    acc += double(confusion[std::make_pair(class_i, class_i)]) / sum;
  }
  std::cout << "ACC: " << (acc / dataset.total_classes) << std::endl;

  return 0;
}
