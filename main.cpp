#include <iostream>

#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <sstream>
#include <string>
#include <fstream>
#include <set>

#include "Tree.cpp"
#include "helpers.cpp"

double baccuracy(Tree tree, std::vector<std::vector<int>> test_data)
{
  std::map<std::pair<int, int>, int> confusion;

  for (auto row : test_data)
  {
    int predict = tree.predict(row);
    int actual = row[row.size() - 1];
    auto key = std::make_pair(predict, actual);
    confusion[key] = confusion.count(key) ? confusion[key] + 1 : 1;
  }

  double accuracy = 0;
  for (int class_i : tree.classes)
  {
    int sum = 0;
    for (int class_j : tree.classes)
    {
      sum += confusion[std::make_pair(class_i, class_j)];
    }
    accuracy += double(confusion[std::make_pair(class_i, class_i)]) / sum;
  }
  return accuracy / tree.classes.size();
};

std::vector<int> prediction(Tree tree, std::vector<std::vector<int>> test_data)
{
  std::vector<int> result;

  for (auto line : test_data)
  {
    int predict = tree.predict(line);
    result.push_back(predict);
  }
  return result;
}

/**
 * build classification tree with maximum depth max_depth (not pruned by default)
 */
Tree fit(std::vector<std::vector<int>> data, int max_depth = 0)
{
  auto dataset = Matrix<int>(data.size(), data.at(0).size(), flatten(data));
  Tree tree(dataset);
  tree.build(max_depth);
  return tree;
}

Tree fit(std::vector<std::vector<int>> data, std::vector<int> classes, int max_depth = 0)
{
  for (int i = 0; i < data.size(); i++)
  {
    data.at(i).push_back(classes.at(i));
  }
  return fit(data, max_depth);
}

int main()
{
  std::cout << "balanced acc.: " << baccuracy(fit(parseCSV("train.txt")), parseCSV("test.txt")) << std::endl;
  return 0;
}
