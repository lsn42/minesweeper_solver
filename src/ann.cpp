// reference: https://blog.csdn.net/qq_37781464/article/details/112030900
#include "ann.hpp"

#include <io.h>

#include <opencv2/opencv.hpp>
#include <string>

#include "util/image.hpp"

namespace minesweeper_solver
{

ANN::ANN()
{
  intptr_t h_file = 0;
  struct _finddata_t file;
  if ((h_file = _findfirst(SAVED_MODEL.c_str(), &file)) == -1)
  {
    printf("no model found, create new model\n");
    this->create_model();
    this->train(TRAIN_DATA);
    this->model->save(SAVED_MODEL);
  }
  else
  {
    printf("load model success\n");
    this->model = cv::ml::StatModel::load<cv::ml::ANN_MLP>(SAVED_MODEL);
  }
}

void ANN::create_model()
{
  cv::Ptr<cv::ml::ANN_MLP> model = cv::ml::ANN_MLP::create();
  //定义模型的层次结构 输入层为14*14*3=588 隐藏层为14*14=196、7*7=49 输出层为11
  cv::Mat layerSizes = (cv::Mat_<int>(1, 4) << 14 * 14 * 3, 14 * 14, 7 * 7, 11);
  model->setLayerSizes(layerSizes);
  //设置参数更新为误差反向传播法
  model->setTrainMethod(cv::ml::ANN_MLP::BACKPROP, 0.001, 0.1);
  //设置激活函数为sigmoid
  model->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM, 1.0, 1.0);
  //设置迭代条件 最大训练次数为100
  model->setTermCriteria(cv::TermCriteria(
    cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 10, 0.0001));

  this->model = model;
}
cv::Mat ANN::predict(cv::Mat in)
{
  cv::Mat out;
  this->model->predict(in, out);
  return out;
}

cv::Ptr<cv::ml::TrainData> ANN::read_train_data(const std::string diretory)
{
  std::vector<std::string> folders;

  std::string path = diretory + "/*";
  intptr_t h_file = 0;
  struct _finddata_t file;

  if ((h_file = _findfirst(path.c_str(), &file)) != -1)
  {
    do
    {
      if (file.attrib == _A_SUBDIR && strcmp(file.name, "..") &&
          strcmp(file.name, "."))
      {
        std::string name = file.name;
        folders.push_back(name);
      }
    } while (!_findnext(h_file, &file));
    _findclose(h_file);
  }

  // initialize by creating zero row for the later operation of row concat
  // zero row will be removed after data loaded
  cv::Mat images = cv::Mat::zeros(1, 14 * 14 * 3, CV_32F);
  cv::Mat labels = cv::Mat::zeros(1, folders.size(), CV_32F);
  for (int i = 0; i < folders.size(); ++i)
  {
    path = diretory + "/" + folders[i] + "/*";
    if ((h_file = _findfirst(path.c_str(), &file)) != -1)
    {
      do
      {
        if (file.attrib != _A_SUBDIR)
        {
          path = diretory + "/" + folders[i] + "/" + file.name;
          cv::Mat img = cv::imread(path);
          img = util::transform_image2(img);
          cv::vconcat(images, img, images);

          cv::Mat r = cv::Mat::zeros(1, folders.size(), CV_32F);
          r.at<float>(0, i) = 1.0;
          cv::vconcat(labels, r, labels);
        }
      } while (!_findnext(h_file, &file));
      _findclose(h_file);
    }
  }
  // remove the first zero row which created while initialize
  images = images.rowRange(1, images.rows);
  labels = labels.rowRange(1, labels.rows);
  printf("images:\trow: %d, col:%d\nlabels:\trow: %d, col:%d\n", images.rows,
    images.cols, labels.rows, labels.cols);
  return cv::ml::TrainData::create(images, cv::ml::ROW_SAMPLE, labels);
}

void ANN::train(std::string diretory)
{
  this->model->train(ANN::read_train_data(diretory));
}
} // namespace minesweeper_solver