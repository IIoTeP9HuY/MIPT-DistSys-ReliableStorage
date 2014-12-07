#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <unordered_map>

#include <boost/make_shared.hpp>

#include "logger/logger.hpp"

#include "../gen-cpp/Server.h"
#include "../gen-cpp/Coordinator.h"
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
  ServerHandler(const string& name): name(name) {
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
    if (!client) {
      throw std::logic_error("Coordinator is not set");
    }
    ViewInfo viewInfo;
    client->ping(viewInfo, 1, name);
    return currentTime;
  }

  void setCoordinator(const string& host) override {
    if (client) {
      transport->close();
    }

    socket = boost::make_shared<TSocket>(host);
    transport = boost::make_shared<TBufferedTransport>(socket);
    protocol = boost::make_shared<TBinaryProtocol>(transport);
    client = boost::make_shared<CoordinatorClient>(protocol);

    try {
      transport->open();
    } catch (TException& tx) {
      cout << "ERROR: " << tx.what() << endl;
    }
  }

  int currentTime;
  const string name;

  boost::shared_ptr<TTransport> socket;
  boost::shared_ptr<TTransport> transport;
  boost::shared_ptr<TProtocol> protocol;
  boost::shared_ptr<CoordinatorClient> client;
};

int main(int argc, char **argv) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " PORT " << "[SERVER_NAME]" << endl;
    return 0;
  }
  const int port = stoi(argv[1]);
  string serverName = "server:" + to_string(port);
  if (argc == 3) {
    serverName = argv[2];
  }

  using boost::shared_ptr;
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
  shared_ptr<ServerHandler> handler(new ServerHandler(serverName));
  shared_ptr<TProcessor> processor(new ServerProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

  cout << "Starting the server..." << endl;
  server.serve();
  cout << "Done." << endl;
  return 0;
}
