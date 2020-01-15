

#include "vectorWorker.h"
#include <v8.h>

void VectorWorker::Execute () {
    try {
        wrapper_->loadModel();
        wrapper_->precomputeWordVectors();
        result_ = wrapper_->getSentenceVector(query_);
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


void VectorWorker::HandleErrorCallback () {
    Nan::HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        Nan::Error(ErrorMessage()),
        Nan::Null()
    };

    callback->Call(2, argv);
}

void VectorWorker::HandleOKCallback () {
    Nan::HandleScope scope;
    v8::Local<v8::Array> result = Nan::New<v8::Array>(result_.size());

    for(unsigned int i = 0; i < result_.size(); i++) {
        result->Set(i, Nan::New<v8::Number>(result_[i]));
    }

    v8::Local<v8::Value> argv[] = {
        Nan::Null(),
        result
    };

    callback->Call(2, argv);
}