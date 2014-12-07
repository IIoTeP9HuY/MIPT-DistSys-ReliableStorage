#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <unordered_map>

#include "logger/logger.hpp"

#include "../gen-cpp/Server.h"
#include "../gen-cpp/ReplicatedStorage_constants.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace logging;

using namespace mipt::distsys::storage;

class ServerHandler : public ServerIf {
public:
  ServerHandler() {
  }

  void put(const string& key, const string& value) override {
  }

  void putBackup(const string& key, const string& value) override {
  }

  void get(string& result, const string& key) override {
  }

  int tick() {
    ++currentTime;
    Log::trace("tick(): ", currentTime);
    return currentTime;
  }

  int currentTime;
};

int main(int argc, char **argv) {
  using boost::shared_ptr;
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
  shared_ptr<ServerHandler> handler(new ServerHandler());
  shared_ptr<TProcessor> processor(new ServerProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(9091));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

  cout << "Starting the server..." << endl;
  server.serve();
  cout << "Done." << endl;
  return 0;
}
