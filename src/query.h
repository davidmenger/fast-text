// query.h
#ifndef QUERY_H
#define QUERY_H

#include <node.h>
#include <node_object_wrap.h>
#include <nan.h>

#include "nodeArgument.h"
#include "wrapper.h"
#include "nnWorker.h"
#include "trainWorker.h"
#include "vectorWorker.h"

class Query : public Nan::ObjectWrap {
    public:
        static NAN_MODULE_INIT(Init) {
            v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
            tpl->SetClassName(Nan::New("Query").ToLocalChecked());
            tpl->InstanceTemplate()->SetInternalFieldCount(1);

            Nan::SetPrototypeMethod(tpl, "nn", Nn);
            Nan::SetPrototypeMethod(tpl, "getSentenceVector", GetSentenceVector);
            Nan::SetPrototypeMethod(tpl, "train", Train);

            constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
            Nan::Set(target, Nan::New("Query").ToLocalChecked(),
            Nan::GetFunction(tpl).ToLocalChecked());
        }

    private:
        explicit Query(std::string modelFilename) :
            wrapper_(new Wrapper(modelFilename))
            {}

        ~Query() {}

        static NAN_METHOD(New) {
            if (info.IsConstructCall()) {
                if (!info[0]->IsString()) {
                    Nan::ThrowError("First argument must be a string");
                    return;
                }

                // v8::String::Utf8Value commandArg(info[0]->ToString());
                Nan::Utf8String commandArg(info[0]);
                std::string command = std::string(*commandArg);

                Query *obj = new Query(command);
                obj->Wrap(info.This());
                info.GetReturnValue().Set(info.This());
            } else {
                const int argc = 1;
                v8::Local<v8::Value> argv[argc] = {info[0]};
                v8::Local<v8::Function> cons = Nan::New(constructor());
                info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
            }
        }


        static NAN_METHOD(Nn) {
            if (!info[0]->IsString()) {
                Nan::ThrowError("query must be a string");
                return;
            }

            if (!info[1]->IsUint32()) {
                Nan::ThrowError("k must be a number");
                return;
            }

            if (!info[2]->IsFunction()) {
                Nan::ThrowError("callback must be a function");
                return;
            }

            // int32_t k = info[1]->ToLocalChecked()->Uint32Value();
            int32_t k = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
            // v8::String::Utf8Value queryArg(info[0]->ToString());
            Nan::Utf8String queryArg(info[0]);
            std::string query = std::string(*queryArg);
            Nan::Callback *callback = new Nan::Callback(info[2].As<v8::Function>());

            Query* obj = Nan::ObjectWrap::Unwrap<Query>(info.Holder());

            Nan::AsyncQueueWorker(new NnWorker(callback, query, k, obj->wrapper_));
        }

        static NAN_METHOD(GetSentenceVector) {
            if (!info[0]->IsString()) {
                Nan::ThrowError("query must be a string");
                return;
            }

            if (!info[1]->IsFunction()) {
                Nan::ThrowError("callback must be a function");
                return;
            }

            // v8::String::Utf8Value queryArg(info[0]->ToString());
            Nan::Utf8String queryArg(info[0]);
            std::string query = std::string(*queryArg);
            Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());

            Query* obj = Nan::ObjectWrap::Unwrap<Query>(info.Holder());

            Nan::AsyncQueueWorker(new VectorWorker(callback, query, obj->wrapper_));
        }

        static NAN_METHOD(Train) {
            if (!info[0]->IsObject()) {
                Nan::ThrowError("options argument must be an object.");
                return;
            }

            if (!info[1]->IsFunction()) {
                Nan::ThrowError("callback must be a function");
                return;
            }


            v8::Local<v8::Object> confObj = v8::Local<v8::Object>::Cast( info[0] );

            NodeArgument::NodeArgument nodeArg;
            NodeArgument::CArgument c_argument;

            try {
                c_argument = nodeArg.ObjectToCArgument( confObj );
            } catch (std::string errorMessage) {
                Nan::ThrowError(errorMessage.c_str());
                return;
            }

            int count = c_argument.argc;
            char** argument = c_argument.argv;

            std::vector<std::string> args;
            args.push_back("-command");
            args.push_back("skipgram");

            for(int j = 0; j < count; j++) {
                args.push_back(argument[j]);
            }

            Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());

            Query* obj = Nan::ObjectWrap::Unwrap<Query>(info.Holder());

            Nan::AsyncQueueWorker(new TrainWorker(callback, args, obj->wrapper_));
        }

        static inline Nan::Persistent<v8::Function> & constructor() {
            static Nan::Persistent<v8::Function> my_constructor;
            return my_constructor;
        }

        Wrapper* wrapper_;
    };

#endif