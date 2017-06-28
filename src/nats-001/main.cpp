#include <nats.h>
#include <iostream>
#include <memory>

int count;

std::shared_ptr<natsOptions> CreateNatsOptions() {
  natsOptions *opts;
  if (natsOptions_Create(&opts) != NATS_OK) {
    return nullptr;
  }
  return std::shared_ptr<natsOptions>(opts, natsOptions_Destroy);
}

std::shared_ptr<natsConnection> Connect(
    const std::shared_ptr<natsOptions> &opts) {
  natsConnection *conn;
  if (natsConnection_Connect(&conn, opts.get()) != NATS_OK) {
    return nullptr;
  }
  return std::shared_ptr<natsConnection>(conn, natsConnection_Destroy);
}

std::shared_ptr<natsSubscription> Subscribe(
    const std::shared_ptr<natsConnection> &conn, const std::string &subject,
    natsMsgHandler on_msg) {
  natsSubscription *sub;
  if (natsConnection_Subscribe(&sub, conn.get(), subject.c_str(), on_msg,
                               nullptr) != NATS_OK) {
    return nullptr;
  }
  return std::shared_ptr<natsSubscription>(sub, natsSubscription_Destroy);
}

void OnError(natsConnection *nc, natsSubscription *sub, natsStatus err,
             void *closure) {
  // TODO
}

void OnMessage(natsConnection *nc, natsSubscription *sub, natsMsg *msg,
               void *closure) {
  if (msg == nullptr) {
    std::cout << "OnMessage ::: timeout" << std::endl;
    return;
  }
  count++;
  std::cout << "OnMessage ::: subject: " << natsMsg_GetSubject(msg)
            << ", data_length: " << natsMsg_GetDataLength(msg) << std::endl;
  natsMsg_Destroy(msg);
}

int main(int argc, char *argv[]) {
  auto opts = CreateNatsOptions();
  if (!opts) {
    std::cout << "ERROR: CreateNatsOptions" << std::endl;
    return 1;
  }
  if (natsOptions_SetErrorHandler(opts.get(), OnError, nullptr) != NATS_OK) {
    std::cout << "ERROR: natsOptions_SetErrorHandler" << std::endl;
    return 1;
  }
  if (natsOptions_SetURL(opts.get(), NATS_DEFAULT_URL) != NATS_OK) {
    std::cout << "ERROR: natsOptions_SetURL" << std::endl;
    return 1;
  }
  auto conn = Connect(opts);
  if (!conn) {
    std::cout << "ERROR: Connect" << std::endl;
    return 1;
  }
  auto sub = Subscribe(conn, "foo", OnMessage);
  if (!sub) {
    std::cout << "ERROR: Subscribe" << std::endl;
    return 1;
  }
  while (true) {
    if (count > 0) {
      break;
    }
    std::cout << "Sleep 1000ms" << std::endl;
    nats_Sleep(1000);
  }
  std::cout << "Exit" << std::endl;
  return 0;
}
