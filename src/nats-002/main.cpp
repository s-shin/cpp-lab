#include <cassert>
#include <memory>

#include "nats/nats.h"

int main(int argc, char *argv[]) {
  natsOptions *opts;
  assert(natsOptions_Create(&opts) == NATS_OK);
  std::shared_ptr<natsOptions> opts_releaser(opts, natsOptions_Destroy);
  assert(natsOptions_SetURL(opts, NATS_DEFAULT_URL) == NATS_OK);

  natsConnection *conn;
  assert(natsConnection_Connect(&conn, opts) == NATS_OK);
  std::shared_ptr<natsConnection> conn_releaser(conn, natsConnection_Destroy);

  assert(natsConnection_PublishString(conn, "foo", "hello!") == NATS_OK);
  return 0;
}
