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

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

struct HostPort {
    HostPort() {
      host = "";
      port = 0;
    }

    HostPort(const string& hostPort) {
      auto hostAndPort = split(hostPort, ':');
      host = hostAndPort[0];
      port = stoi(hostAndPort[1]);
    }

    string host;
    int port;
};

class ServerHandler : public ServerIf {
public:
  ServerHandler(const string& myHostPort): name(myHostPort) {
  }

  ServerHandler(const string& myHostPort, const string& coordHostPort)
    : ServerHandler(myHostPort)
  {
    setCoordinator(coordHostPort);
  }

  void put(const string& key, const string& value) override {
    if (view.primary != name) {
      Log::error(name, ": ", "put(): ", "Server is not primary");
      auto e = IncorrectOperationException();
      e.why = "Server is not primary";
      throw e;
    }
    storage[key] = value;
    if (!view.backup.empty()) {
      backup.client->putBackup(key, value);
    }
  }

  void putBackup(const string& key, const string& value) override {
    if (view.backup != name) {
      Log::error(name, ": ", "putBackup(): ", "Server is not backup");
      auto e = IncorrectOperationException();
      e.why = "Server is not backup";
      throw e;
    }
    storage[key] = value;
  }

  void get(string& result, const string& key) override {
    if (view.primary != name) {
      Log::error(name, ": ", "get(): ", "Server is not primary");
      auto e = IncorrectOperationException();
      e.why = "Server is not primary";
      throw e;
    }
    result = storage[key];
  }

  int tick() {
    ++currentTime;
    Log::trace(name, ": ", "tick(): ", currentTime);
    if (!coordinator.connected()) {
      throw std::logic_error("Coordinator is not set");
    }
    ViewInfo viewInfo;
    coordinator.client->ping(viewInfo, view.view, name);

    updatePrimary(viewInfo);
    updateBackup(viewInfo);

    view.view = viewInfo.view;

    return currentTime;
  }

  void updatePrimary(const ViewInfo& viewInfo) {
    if (view.primary == viewInfo.primary) {
      return;
    }

    if (viewInfo.primary == name) {
      becomePrimary(viewInfo);
    } else {
      view.primary = viewInfo.primary;
    }
  }

  void updateBackup(const ViewInfo& viewInfo) {
    if (view.backup == viewInfo.backup) {
      return;
    }

    if (viewInfo.backup == name) {
      becomeBackup(viewInfo);
    } else {
      if (isPrimary()) {
        setBackup(viewInfo.backup);
      }
    }
  }

  bool isPrimary() const {
    return view.primary == name;
  }

  bool isBackup() const {
    return view.backup == name;
  }

  void becomePrimary(const ViewInfo& viewInfo) {
    Log::info(name, ": ", "becomePrimary()");
    view.primary = viewInfo.primary;
    if (!viewInfo.backup.empty()) {
      setBackup(viewInfo.backup);
    }
  }

  void becomeBackup(const ViewInfo& viewInfo) {
    Log::info(name, ": ", "becomeBackup()");
    view.backup = viewInfo.backup;
  }

  void setCoordinator(const string& hostPort) override {
    Log::info(name, ": ", "setCoordinator('", hostPort, "')");

    if (coordinator.connected()) {
      coordinator.close();
    }
    coordinator.open(hostPort);
  }

  void copyDataToBackup() {
    for (const auto& data : storage) {
      backup.client->putBackup(data.first, data.second);
    }
  }

  void setBackup(const string& hostPort) {
    Log::info(name, ": ", "setBackup('", hostPort, "')");

    if (backup.connected()) {
      backup.close();
    }
    if (!hostPort.empty()) {
      backup.open(hostPort);
      copyDataToBackup();
    }
    view.backup = hostPort;
  }

  template<typename Client>
  struct Connection {
    void open(const string& hostPort_) {
      hostPort = HostPort(hostPort_);
      socket = boost::make_shared<TSocket>(hostPort.host, hostPort.port);
      transport = boost::make_shared<TBufferedTransport>(socket);
      protocol = boost::make_shared<TBinaryProtocol>(transport);
      client = boost::make_shared<Client>(protocol);
      try {
        transport->open();
      } catch (TException& tx) {
        cout << "ERROR: " << tx.what() << endl;
      }
    }

    bool connected() const {
      return client != nullptr;
    }

    void close() {
      transport->close();
      client.reset();
      protocol.reset();
      transport.reset();
      socket.reset();
      hostPort = HostPort();
    }

    HostPort hostPort;

    boost::shared_ptr<TTransport> socket;
    boost::shared_ptr<TTransport> transport;
    boost::shared_ptr<TProtocol> protocol;
    boost::shared_ptr<Client> client;
  };

  int currentTime;
  const string name;
  ViewInfo view;

  std::unordered_map<string, string> storage;

  Connection<CoordinatorClient> coordinator;
  Connection<ServerClient> backup;
};

int main(int argc, char **argv) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " PORT" << endl;
    return 0;
  }
  const int port = stoi(argv[1]);
  string serverName = "0.0.0.0:" + to_string(port);

  using boost::shared_ptr;
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
  shared_ptr<ServerHandler> handler(new ServerHandler(serverName));
  shared_ptr<TProcessor> processor(new ServerProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

  TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);

  cout << "Starting server '" << serverName << "' on port " << port << endl;
  server.serve();
  cout << "Done." << endl;
  return 0;
}
