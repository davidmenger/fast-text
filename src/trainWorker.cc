

#include "trainWorker.h"
#include <v8.h>

void TrainWorker::Execute () {
    try {
        wrapper_->train(query_);
    } catch (std::string errorMessage) {
        this->SetErrorMessage(errorMessage.c_str());
    } catch (const char * str) {
        this->SetErrorMessage(str);
    } catch (const std::exception& e) {
        this->SetErrorMessage(e.what());
    } catch  (const std::invalid_argument& ia) {
        this->SetErrorMessage(ia.what());
    }
}


void TrainWorker::HandleErrorCallback () {
    Nan::HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        Nan::Error(ErrorMessage())
    };

    callback->Call(1, argv);
}

void TrainWorker::HandleOKCallback () {
    Nan::HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        Nan::Null()
    };

    callback->Call(1, argv);
}