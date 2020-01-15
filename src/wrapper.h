

#ifndef WRAPPER_H
#define WRAPPER_H

// #include <time.h>

#include <atomic>
#include <memory>
#include <set>
#include  <mutex>

#include "../lib/src/fasttext.h"

using fasttext::Args;
using fasttext::Dictionary;
using fasttext::Matrix;
using fasttext::QMatrix;
using fasttext::Model;
using fasttext::Vector;
using fasttext::real;

struct PredictResult {
    std::string label;
    double value;
};

class Wrapper {
    protected:
        std::shared_ptr<Args> args_;
        std::shared_ptr<Dictionary> dict_;

        std::shared_ptr<Matrix> input_;
        std::shared_ptr<Matrix> output_;

        std::shared_ptr<QMatrix> qinput_;
        std::shared_ptr<QMatrix> qoutput_;

        std::shared_ptr<Model> model_;
        Matrix wordVectors_;


        std::atomic<int64_t> tokenCount_;
        std::atomic<real> loss_;
        clock_t start_;

        void signModel(std::ostream&);
        bool checkModel(std::istream&);

        std::vector<PredictResult> findNN(const Vector&, int32_t,
                    const std::set<std::string>&);

        void loadModel(std::istream&);
        void loadVectors(std::string);
        void trainThread(int32_t);

        void supervised(
            Model&,
            real,
            const std::vector<int32_t>&,
            const std::vector<int32_t>&);
        void cbow(Model&, real, const std::vector<int32_t>&);
        void skipgram(Model&, real, const std::vector<int32_t>&);

        bool quant_;
        std::string modelFilename_;
        std::mutex mtx_;
        std::mutex precomputeMtx_;

        bool isLoaded_;
        bool isPrecomputed_;

        void startThreads();
    public:
        Wrapper(std::string modelFilename);

        void getVector(Vector&, const std::string&);

        std::vector<PredictResult> predict(std::string sentence, int32_t k);
        std::vector<PredictResult> nn(std::string query, int32_t k);

        void train(const std::vector<std::string> args);

        void precomputeWordVectors();
        void loadModel();

        std::vector<double> getSentenceVector(std::string);
        void getWordVector(Vector&, const std::string&) const;
        void addInputVector(Vector&, int32_t) const;

};

#endif
