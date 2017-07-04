#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "nats/adapters/libuv.h"
#include "nats/nats.h"
#include "uv.h"

namespace nats {

template <class Fn, class Key = uint32_t>
class CallbackRegistory {
 public:
  CallbackRegistory() {}

  void Register(const Key &key, const Fn &fn) { cbs_.emplace(key, fn); }

  bool Unregister(const Key &key) { return cbs_.erase(key) == 1; }

  bool Exists(const Key &key) { return cbs_.find(key) != cbs_.end(); }

  template <class... Args>
  bool Call(const Key &key, Args... args) {
    auto it = cbs_.find(key);
    if (it != cbs_.end()) {
      it->second(args...);
      return true;
    }
    return false;
  }

 private:
  std::unordered_map<Key, Fn> cbs_;
};

class Message {
 public:
  Message(natsMsg *msg) : msg_(msg, natsMsg_Destroy) {}

  const char *GetSubject() { return natsMsg_GetSubject(msg_.get()); }
  int GetDataLength() { return natsMsg_GetDataLength(msg_.get()); }
  const char *GetData() { return natsMsg_GetData(msg_.get()); }

 private:
  std::shared_ptr<natsMsg> msg_;
};

class Subscription {
 public:
  Subscription(natsSubscription *sub) : sub_(sub, natsSubscription_Destroy) {}

  natsSubscription *nats_sub() const { return sub_.get(); }

 private:
  std::shared_ptr<natsSubscription> sub_;
};

class Connection {
 public:
  using OnMessageFunc =
      std::function<void(const std::shared_ptr<Message> &msg)>;

  struct Config {
    std::string url;
  };

  Connection(const Config &config) : Connection(config, nullptr) {}

  Connection(const Config &config, uv_loop_t *loop)
      : nats_ok_(true), nats_status_(), opts_(), conn_() {
    if (loop != nullptr) {
      natsLibuv_Init();
      natsLibuv_SetThreadLocalLoop(loop);
    }

    natsOptions *opts;
    if (!MakeSureOfNatsOK(natsOptions_Create(&opts))) {
      return;
    }
    opts_.reset(opts, natsOptions_Destroy);

    if (loop != nullptr) {
      if (!MakeSureOfNatsOK(natsOptions_SetEventLoop(
              opts, static_cast<void *>(loop), natsLibuv_Attach, natsLibuv_Read,
              natsLibuv_Write, natsLibuv_Detach))) {
        return;
      }
    }

    if (!MakeSureOfNatsOK(natsOptions_SetURL(opts, config.url.c_str()))) {
      return;
    }

    natsConnection *conn;
    if (!MakeSureOfNatsOK(natsConnection_Connect(&conn, opts))) {
      return;
    }
    conn_.reset(conn, natsConnection_Destroy);
  }

  ~Connection() {
    opts_.reset();
    for (const auto &sub : subs_) {
      OnMessageFuncRegistory().Unregister(sub->nats_sub());
    }
    subs_.clear();
    conn_.reset();
  }

  explicit operator bool() const { return nats_ok_; }

  std::string error() { return natsStatus_GetText(nats_status_); }

  std::shared_ptr<Subscription> Subscribe(const std::string &subject,
                                              const OnMessageFunc &on_msg) {
    natsSubscription *sub;
    if (!MakeSureOfNatsOK(natsConnection_Subscribe(
            &sub, conn_.get(), subject.c_str(), DispatchMessage, nullptr))) {
      return nullptr;
    }
    OnMessageFuncRegistory().Register(sub, on_msg);
    auto nsub = std::make_shared<Subscription>(sub);
    subs_.emplace_back(nsub);
    return nsub;
  }

  static void DispatchMessage(natsConnection *nc, natsSubscription *sub,
                              natsMsg *msg, void *closure) {
    OnMessageFuncRegistory().Call(sub, std::make_shared<Message>(msg));
  }

  bool Unsubscribe(const std::shared_ptr<Subscription> &sub) {
    natsSubscription_Unsubscribe(sub->nats_sub());
    OnMessageFuncRegistory().Unregister(sub->nats_sub());
    subs_.erase(std::remove(subs_.begin(), subs_.end(), sub), subs_.end());
    return true;
  }

  bool PublishString(const std::string &subject, const std::string &msg) {
    return MakeSureOfNatsOK(natsConnection_PublishString(
        conn_.get(), subject.c_str(), msg.c_str()));
  }

 private:
  bool MakeSureOfNatsOK(natsStatus status) {
    nats_status_ = status;
    nats_ok_ = status == NATS_OK;
    return nats_ok_;
  }

  static CallbackRegistory<OnMessageFunc, natsSubscription *>
      &OnMessageFuncRegistory() {
    static CallbackRegistory<OnMessageFunc, natsSubscription *> registory;
    return registory;
  }

  bool nats_ok_;
  natsStatus nats_status_;
  std::shared_ptr<natsOptions> opts_;
  std::shared_ptr<natsConnection> conn_;
  std::vector<std::shared_ptr<Subscription>> subs_;
};

}  // namespace nats

int main(int argc, char *argv[]) {
  uv_loop_t *loop = uv_default_loop();

  nats::Connection conn({NATS_DEFAULT_URL}, loop);
  if (!conn) {
    std::cout << "connect error: " << conn.error() << std::endl;
    return 1;
  }
  std::cout << "connect ok" << std::endl;

  bool next = false;
  auto sub =
      conn.Subscribe("foo", [&next](const std::shared_ptr<nats::Message> &msg) {
        std::string str(msg->GetData(), msg->GetDataLength());
        std::cout << "foo ::: " << str << std::endl;
        next = true;
      });
  if (sub == nullptr) {
    std::cout << "subscribe failed: " << conn.error() << std::endl;
    return 1;
  }

  conn.PublishString("foo", "hoge");

  while (!next) {
    uv_run(loop, UV_RUN_NOWAIT);
  }

  conn.Unsubscribe(sub);
  conn.PublishString("foo", "fuga");

  auto t = nats_Now();
  while (nats_Now() - t <= 2000) {
    uv_run(loop, UV_RUN_NOWAIT);
  }

  std::cout << "end" << std::endl;
  return 0;
}
