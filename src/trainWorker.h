
#ifndef TRAIN_WORKER_H
#define TRAIN_WORKER_H

#include <nan.h>
#include "wrapper.h"

class TrainWorker : public Nan::AsyncWorker {
    public:
        TrainWorker (Nan::Callback *callback, std::vector<std::string> query, Wrapper *wrapper)
            : Nan::AsyncWorker(callback),
                query_(query),
                wrapper_(wrapper) {};

        ~TrainWorker () {};

        void Execute ();
        void HandleOKCallback ();
        void HandleErrorCallback ();

    private:
        std::vector<std::string> query_;
        Wrapper *wrapper_;
};

#endif