
#ifndef VECTOR_WORKER_H
#define VECTOR_WORKER_H

#include <nan.h>
#include "wrapper.h"

class VectorWorker : public Nan::AsyncWorker {
    public:
        VectorWorker (Nan::Callback *callback, std::string query, Wrapper *wrapper)
            : Nan::AsyncWorker(callback),
                query_(query),
                wrapper_(wrapper),
                result_() {};

        ~VectorWorker () {};

        void Execute ();
        void HandleOKCallback ();
        void HandleErrorCallback ();

    private:
        std::string query_;
        Wrapper *wrapper_;
        std::vector<double> result_;
};

#endif