#include "nan.h"
#include "spawn_as_admin.h"

namespace spawn_as_admin {
namespace {

using namespace v8;

class Worker : public Nan::AsyncWorker {
  Session session;
  int exit_code;

public:
  Worker(Nan::Callback *callback, Session session) :
    Nan::AsyncWorker(callback),
    session(session),
    exit_code(-1) {}

  void Execute() {
    exit_code = spawn_as_admin::FinishSpawnAsAdmin(&session);
  }

  void HandleOKCallback() {
    Local<Value> argv[] = {Nan::New<Integer>(exit_code)};
    callback->Call(1, argv);
  }
};

void SpawnAsAdmin(const Nan::FunctionCallbackInfo<Value>& info) {
  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("Command must be a string");
    return;
  }

  std::string command(*String::Utf8Value(info[0]));

  if (!info[1]->IsArray()) {
    Nan::ThrowTypeError("Arguments must be an array");
    return;
  }

  Local<Array> js_args = Local<Array>::Cast(info[1]);
  std::vector<std::string> args(js_args->Length());
  for (uint32_t i = 0; i < args.size(); ++i) {
    Local<Value> js_arg = js_args->Get(i);

    if (!js_arg->IsString()) {
      Nan::ThrowTypeError("Arguments must be an array of strings");
      return;
    }

    args.push_back(std::string(*String::Utf8Value(js_arg)));
  }

  if (!info[2]->IsFunction()) {
    Nan::ThrowTypeError("Callback must be a function");
    return;
  }

  bool admin = true;
  if (info[3]->IsFalse()) admin = false;

  Session session = spawn_as_admin::StartSpawnAsAdmin(command, args, admin);
  if (session.pid == -1) return;

  Local<Object> result = Nan::New<Object>();
  result->Set(Nan::New("pid").ToLocalChecked(), Nan::New<Integer>(session.pid));
  result->Set(Nan::New("stdin").ToLocalChecked(), Nan::New<Integer>(session.stdin_file_descriptor));
  result->Set(Nan::New("stdout").ToLocalChecked(), Nan::New<Integer>(session.stdout_file_descriptor));
  info.GetReturnValue().Set(result);

  Nan::AsyncQueueWorker(new Worker(new Nan::Callback(info[2].As<Function>()), session));
}

void Init(Handle<Object> exports) {
  Nan::SetMethod(exports, "spawnAsAdmin", SpawnAsAdmin);
}

NODE_MODULE(runas, Init)

}  // namespace
}  // namespace spawn_as_admin
