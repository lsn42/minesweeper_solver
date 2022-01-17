// reference: https://blog.csdn.net/qq_37781464/article/details/112030900
#ifndef MINESWEEPER_SOLVER_ANN_HPP
#define MINESWEEPER_SOLVER_ANN_HPP
#include <opencv2/opencv.hpp>
#include <string>

static const std::string TRAIN_DATA = "../data/1";
static const std::string SAVED_MODEL = "../model/mnist_ann.xml";

namespace minesweeper_solver
{
class ANN
{
 public:
  cv::Ptr<cv::ml::ANN_MLP> model;

  ANN();
  void create_model();
  static cv::Ptr<cv::ml::TrainData> read_train_data(const std::string dir);
  void train(std::string diretory);
  cv::Mat predict(cv::Mat);
};
} // namespace minesweeper_solver
#endif