#include <cassert>
#include <iostream>
#include <memory>

#include "nats/nats.h"

int count = 0;
natsStatus error = NATS_OK;

void OnError(natsConnection *nc, natsSubscription *sub, natsStatus err,
             void *closure) {
  std::cout << "OnError ::: error: " << natsStatus_GetText(err) << std::endl;
  error = err;
}

void OnMessage(natsConnection *nc, natsSubscription *sub, natsMsg *msg,
               void *closure) {
  if (msg == nullptr) {
    std::cout << "OnMessage ::: timeout" << std::endl;
    return;
  }
  std::string strMsg(natsMsg_GetData(msg), natsMsg_GetDataLength(msg));
  std::cout << "OnMessage ::: subject: " << natsMsg_GetSubject(msg)
            << ", data_length: " << natsMsg_GetDataLength(msg)
            << ", data: " << strMsg << std::endl;
  natsMsg_Destroy(msg);
  count++;
}

int main(int argc, char *argv[]) {
  natsOptions *opts;
  assert(natsOptions_Create(&opts) == NATS_OK);
  std::shared_ptr<natsOptions> opts_releaser(opts, natsOptions_Destroy);
  assert(natsOptions_SetURL(opts, NATS_DEFAULT_URL) == NATS_OK);
  assert(natsOptions_SetErrorHandler(opts, OnError, nullptr) == NATS_OK);

  natsConnection *conn;
  assert(natsConnection_Connect(&conn, opts) == NATS_OK);
  std::shared_ptr<natsConnection> conn_releaser(conn, natsConnection_Destroy);

  natsSubscription *sub;
  assert(natsConnection_Subscribe(&sub, conn, "foo", OnMessage, nullptr) ==
         NATS_OK);
  std::shared_ptr<natsSubscription> sub_releaser(sub, natsSubscription_Destroy);

  while (error == NATS_OK) {
    if (count > 0) {
      break;
    }
    std::cout << "Sleep 1000ms" << std::endl;
    nats_Sleep(1000);
  }

  return 0;
}
