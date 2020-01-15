


#include "wrapper.h"

#include <math.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>


constexpr int32_t FASTTEXT_VERSION = 12; /* Version 1b */
constexpr int32_t FASTTEXT_FILEFORMAT_MAGIC_INT32 = 793712314;

using fasttext::model_name;
using fasttext::entry_type;

Wrapper::Wrapper(std::string modelFilename)
    : quant_(false),
        modelFilename_(modelFilename),
        isLoaded_(false),
        isPrecomputed_(false) {}

void Wrapper::getVector(Vector& vec, const std::string& word) {
    const std::vector<int32_t>& ngrams = dict_->getSubwords(word);
    vec.zero();
    for (auto it = ngrams.begin(); it != ngrams.end(); ++it) {
        vec.addRow(*input_, *it);
    }
    if (ngrams.size() > 0) {
        vec.mul(1.0 / ngrams.size());
    }
}

bool Wrapper::checkModel(std::istream& in) {
    int32_t magic;
    int32_t version;
    in.read((char*)&(magic), sizeof(int32_t));
    if (magic != FASTTEXT_FILEFORMAT_MAGIC_INT32) {
        return false;
    }
    in.read((char*)&(version), sizeof(int32_t));
    if (version != FASTTEXT_VERSION) {
        return false;
    }
    return true;
}

void Wrapper::signModel(std::ostream& out) {
    const int32_t magic = FASTTEXT_FILEFORMAT_MAGIC_INT32;
    const int32_t version = FASTTEXT_VERSION;
    out.write((char*)&(magic), sizeof(int32_t));
    out.write((char*)&(version), sizeof(int32_t));
}

void Wrapper::loadModel() {
    if (isLoaded_) {
        return;
    }
    mtx_.lock();
    if (isLoaded_) {
        mtx_.unlock();
        return;
    }
    std::ifstream ifs(this->modelFilename_, std::ifstream::binary);
    if (!ifs.is_open()) {
        throw "Model file cannot be opened for loading!";
    }
    if (!checkModel(ifs)) {
        throw "Model file has wrong file format!";
    }
    loadModel(ifs);
    ifs.close();
    isLoaded_ = true;
    mtx_.unlock();
}

void Wrapper::loadModel(std::istream& in) {
    args_ = std::make_shared<Args>();
    dict_ = std::make_shared<Dictionary>(args_);
    input_ = std::make_shared<Matrix>();
    output_ = std::make_shared<Matrix>();
    qinput_ = std::make_shared<QMatrix>();
    qoutput_ = std::make_shared<QMatrix>();
    args_->load(in);

    dict_->load(in);

    bool quant_input;
    in.read((char*) &quant_input, sizeof(bool));
    if (quant_input) {
        quant_ = true;
        qinput_->load(in);
    } else {
        input_->load(in);
    }

    in.read((char*) &args_->qout, sizeof(bool));
    if (quant_ && args_->qout) {
        qoutput_->load(in);
    } else {
        output_->load(in);
    }

    model_ = std::make_shared<Model>(input_, output_, args_, 0);
    model_->quant_ = quant_;
    model_->setQuantizePointer(qinput_, qoutput_, args_->qout);

    if (args_->model == model_name::sup) {
        model_->setTargetCounts(dict_->getCounts(entry_type::label));
    } else {
        model_->setTargetCounts(dict_->getCounts(entry_type::word));
    }
}

void Wrapper::precomputeWordVectors() {
    if (isPrecomputed_) {
        return;
    }
    precomputeMtx_.lock();
    if (isPrecomputed_) {
        precomputeMtx_.unlock();
        return;
    }
    Matrix wordVectors(dict_->nwords(), args_->dim);
    wordVectors_ = wordVectors;
    Vector vec(args_->dim);
    wordVectors_.zero();
    for (int32_t i = 0; i < dict_->nwords(); i++) {
        std::string word = dict_->getWord(i);
        getVector(vec, word);
        real norm = vec.norm();
        wordVectors_.addRow(vec, i, 1.0 / norm);
    }
    isPrecomputed_ = true;
    precomputeMtx_.unlock();
}

std::vector<PredictResult> Wrapper::findNN(const Vector& queryVec, int32_t k,
        const std::set<std::string>& banSet) {

    real queryNorm = queryVec.norm();
    if (std::abs(queryNorm) < 1e-8) {
        queryNorm = 1;
    }
    std::priority_queue<std::pair<real, std::string>> heap;
    Vector vec(args_->dim);
    for (int32_t i = 0; i < dict_->nwords(); i++) {
        std::string word = dict_->getWord(i);
        real dp = wordVectors_.dotRow(queryVec, i);
        heap.push(std::make_pair(dp / queryNorm, word));
    }

    PredictResult response;
    std::vector<PredictResult> arr;
    int32_t i = 0;
    while (i < k && heap.size() > 0) {
        auto it = banSet.find(heap.top().second);
        if (it == banSet.end()) {
            response = { heap.top().second, exp(heap.top().first) };
            arr.push_back(response);
            i++;
        }
        heap.pop();
    }
    return arr;
}

std::vector<PredictResult> Wrapper::nn(std::string query, int32_t k) {
    Vector queryVec(args_->dim);
    std::set<std::string> banSet;
    banSet.clear();
    banSet.insert(query);
    getVector(queryVec, query);
    return findNN(queryVec, k, banSet);
}

void Wrapper::loadVectors(std::string filename) {
  std::ifstream in(filename);
  std::vector<std::string> words;
  std::shared_ptr<Matrix> mat; // temp. matrix for pretrained vectors
  int64_t n, dim;
  if (!in.is_open()) {
    throw std::invalid_argument(filename + " cannot be opened for loading!");
  }
  in >> n >> dim;
  if (dim != args_->dim) {
    throw std::invalid_argument(
        "Dimension of pretrained vectors (" + std::to_string(dim) +
        ") does not match dimension (" + std::to_string(args_->dim) + ")!");
  }
  mat = std::make_shared<Matrix>(n, dim);
  for (size_t i = 0; i < n; i++) {
    std::string word;
    in >> word;
    words.push_back(word);
    dict_->add(word);
    for (size_t j = 0; j < dim; j++) {
      in >> mat->data_[i * dim + j];
    }
  }
  in.close();

  dict_->threshold(1, 0);
  input_ = std::make_shared<Matrix>(dict_->nwords()+args_->bucket, args_->dim);
  input_->uniform(1.0 / args_->dim);

  for (size_t i = 0; i < n; i++) {
    int32_t idx = dict_->getId(words[i]);
    if (idx < 0 || idx >= dict_->nwords()) continue;
    for (size_t j = 0; j < dim; j++) {
      input_->data_[idx * dim + j] = mat->data_[i * dim + j];
    }
  }
}

std::vector<double> Wrapper::getSentenceVector(std::string sentence) {


  Vector svec(args_->dim);
  svec.zero();
  if (args_->model == model_name::sup) {
    std::vector<int32_t> line, labels;
    std::istringstream in(sentence);
    dict_->getLine(in, line, labels, model_->rng);
    for (int32_t i = 0; i < line.size(); i++) {
      addInputVector(svec, line[i]);
    }
    if (!line.empty()) {
      svec.mul(1.0 / line.size());
    }
  } else {
    Vector vec(args_->dim);
    std::istringstream iss(sentence);
    std::string word;
    int32_t count = 0;
    while (iss >> word) {
      getWordVector(vec, word);
      real norm = vec.norm();
      if (norm > 0) {
        vec.mul(1.0 / norm);
        svec.addVector(vec);
        count++;
      }
    }
    if (count > 0) {
      svec.mul(1.0 / count);
    }
  }
  std::vector<double> result;
  for(unsigned int i = 0; i < svec.size(); i++) {
    result.push_back(svec[i]);
  }
  return result;
}

void Wrapper::addInputVector(Vector& vec, int32_t ind) const {
  if (quant_) {
    vec.addRow(*qinput_, ind);
  } else {
    vec.addRow(*input_, ind);
  }
}

void Wrapper::getWordVector(Vector& vec, const std::string& word) const {
  const std::vector<int32_t>& ngrams = dict_->getSubwords(word);
  vec.zero();
  for (int i = 0; i < ngrams.size(); i ++) {
    addInputVector(vec, ngrams[i]);
  }
  if (ngrams.size() > 0) {
    vec.mul(1.0 / ngrams.size());
  }
}

void Wrapper::trainThread(int32_t threadId) {
  std::ifstream ifs(args_->input);
  if (!ifs.is_open()) {
      throw "Training data cannot be opened for loading!";
  }
  fasttext::utils::seek(ifs, threadId * fasttext::utils::size(ifs) / args_->thread);

  Model model(input_, output_, args_, threadId);
  if (args_->model == model_name::sup) {
    model.setTargetCounts(dict_->getCounts(entry_type::label));
  } else {
    model.setTargetCounts(dict_->getCounts(entry_type::word));
  }


  const int64_t ntokens = dict_->ntokens();
  int64_t localTokenCount = 0;
  std::vector<int32_t> line, labels;
  while (tokenCount_ < args_->epoch * ntokens) {
    real progress = real(tokenCount_) / (args_->epoch * ntokens);
    real lr = args_->lr * (1.0 - progress);
    if (args_->model == model_name::sup) {
      localTokenCount += dict_->getLine(ifs, line, labels, model.rng);
      supervised(model, lr, line, labels);
    } else if (args_->model == model_name::cbow) {
      localTokenCount += dict_->getLine(ifs, line, model.rng);
      cbow(model, lr, line);
    } else if (args_->model == model_name::sg) {
      localTokenCount += dict_->getLine(ifs, line, model.rng);
      skipgram(model, lr, line);
    }
    if (localTokenCount > args_->lrUpdateRate) {
      tokenCount_ += localTokenCount;
      localTokenCount = 0;
      if (threadId == 0) loss_ = model.getLoss();
    }
  }
  ifs.close();
}

void Wrapper::supervised(
    Model& model,
    real lr,
    const std::vector<int32_t>& line,
    const std::vector<int32_t>& labels) {
  if (labels.size() == 0 || line.size() == 0) return;
  std::uniform_int_distribution<> uniform(0, labels.size() - 1);
  int32_t i = uniform(model.rng);
  model.update(line, labels[i], lr);
}

void Wrapper::cbow(Model& model, real lr,
                    const std::vector<int32_t>& line) {
  std::vector<int32_t> bow;
  std::uniform_int_distribution<> uniform(1, args_->ws);
  for (int32_t w = 0; w < line.size(); w++) {
    int32_t boundary = uniform(model.rng);
    bow.clear();
    for (int32_t c = -boundary; c <= boundary; c++) {
      if (c != 0 && w + c >= 0 && w + c < line.size()) {
        const std::vector<int32_t>& ngrams = dict_->getSubwords(line[w + c]);
        bow.insert(bow.end(), ngrams.cbegin(), ngrams.cend());
      }
    }
    model.update(bow, line[w], lr);
  }
}

void Wrapper::skipgram(Model& model, real lr,
                        const std::vector<int32_t>& line) {
  std::uniform_int_distribution<> uniform(1, args_->ws);
  for (int32_t w = 0; w < line.size(); w++) {
    int32_t boundary = uniform(model.rng);
    const std::vector<int32_t>& ngrams = dict_->getSubwords(line[w]);
    for (int32_t c = -boundary; c <= boundary; c++) {
      if (c != 0 && w + c >= 0 && w + c < line.size()) {
        model.update(ngrams, line[w + c], lr);
      }
    }
  }
}

void Wrapper::train(const std::vector<std::string> args) {
    qinput_ = std::make_shared<QMatrix>();
    qoutput_ = std::make_shared<QMatrix>();

    isLoaded_ = true;
    isPrecomputed_ = true;

    // set up args
    args_ = std::make_shared<Args>();
    args_->input = this->modelFilename_;
    args_->verbose = 0;
	args_->parseArgs(args);

    dict_ = std::make_shared<Dictionary>(args_);
    if (args_->input == "-") {
        // manage expectations
        throw std::invalid_argument("Cannot use stdin for training!");
    }
    std::ifstream ifs(args_->input);
    if (!ifs.is_open()) {
        throw std::invalid_argument(
            args_->input + " cannot be opened for training!");
    }
    dict_->readFromFile(ifs);
    ifs.close();

    if (args_->pretrainedVectors.size() != 0) {
        loadVectors(args_->pretrainedVectors);
    } else {
        input_ = std::make_shared<Matrix>(dict_->nwords()+args_->bucket, args_->dim);
        input_->uniform(1.0 / args_->dim);
    }

    if (args_->model == model_name::sup) {
        output_ = std::make_shared<Matrix>(dict_->nlabels(), args_->dim);
    } else {
        output_ = std::make_shared<Matrix>(dict_->nwords(), args_->dim);
    }
    output_->zero();
    startThreads();
    model_ = std::make_shared<Model>(input_, output_, args_, 0);
    if (args_->model == model_name::sup) {
        model_->setTargetCounts(dict_->getCounts(entry_type::label));
    } else {
        model_->setTargetCounts(dict_->getCounts(entry_type::word));
    }
}

void Wrapper::startThreads() {
  start_ = clock();
  tokenCount_ = 0;
  loss_ = -1;
  std::vector<std::thread> threads;
  for (int32_t i = 0; i < args_->thread; i++) {
    threads.push_back(std::thread([=]() { trainThread(i); }));
  }
  const int64_t ntokens = dict_->ntokens();
  // Same condition as trainThread
  while (tokenCount_ < args_->epoch * ntokens) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  for (int32_t i = 0; i < args_->thread; i++) {
    threads[i].join();
  }
}

std::vector<PredictResult> Wrapper::predict (std::string sentence, int32_t k) {

    std::vector<PredictResult> arr;
    std::vector<int32_t> words, labels;
    std::istringstream in(sentence);

    dict_->getLine(in, words, labels, model_->rng);

    // std::cerr << "Got line!" << std::endl;

    if (words.empty()) {
        return arr;
    }

    Vector hidden(args_->dim);
    Vector output(dict_->nlabels());
    std::vector<std::pair<real,int32_t>> modelPredictions;
    model_->predict(words, k, modelPredictions, hidden, output);

    PredictResult response;

    for (auto it = modelPredictions.cbegin(); it != modelPredictions.cend(); it++) {
        response = { dict_->getLabel(it->second), exp(it->first) };
        arr.push_back(response);
    }

    return arr;
}
