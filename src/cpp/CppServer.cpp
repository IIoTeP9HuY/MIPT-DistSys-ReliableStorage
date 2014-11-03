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

#include "../gen-cpp/Coordinator.h"
#include "../gen-cpp/ReplicatedStorage_constants.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace mipt::distsys::storage;

class CoordinatorHandler : public CoordinatorIf {
public:
  CoordinatorHandler() {
    deadPings = g_ReplicatedStorage_constants.DEAD_PINGS;
    currentTime = 0;
    currentViewInfo.view = 0;
    currentViewInfo.primary = "";
    currentViewInfo.backup = "";
    cerr << "Running server with " << "deadPings = " << deadPings << endl;
  }

  void ping(ViewInfo& viewInfo, const int32_t viewNum, const string& name) {
    cerr << "ping(" << viewNum << ", " << name << ")" << endl;
    cerr << "before view(" << currentViewInfo.view << ", " << currentViewInfo.primary << ", " << currentViewInfo.backup << ")" << endl;
    ViewInfo viewBefore = currentViewInfo;
    updateReplica(name, viewNum);
    restore();
    if (viewBefore.primary != currentViewInfo.primary || viewBefore.backup != currentViewInfo.backup) {
      ++currentViewInfo.view;
    }
    viewInfo = currentViewInfo;
    cerr << "after  view(" << currentViewInfo.view << ", " << currentViewInfo.primary << ", " << currentViewInfo.backup << ")" << endl;
  }

  void primary(string& result) {
    cerr << "primary(): " << currentViewInfo.primary << endl;
    result = currentViewInfo.primary;
  }

  int tick() {
    cerr << "tick()" << endl;
    ++currentTime;
    ViewInfo viewBefore = currentViewInfo;
    handleDeadReplicas();
    restore();
    if (viewBefore.primary != currentViewInfo.primary || viewBefore.backup != currentViewInfo.backup) {
      ++currentViewInfo.view;
    }
    return currentTime;
  }

protected:
  struct ReplicaInfo {
    ReplicaInfo() {
      lastPingTime = 0;
    }

    int lastPingTime;
    int viewNum;
  };

  void handleDeadReplicas() {
    // cerr << "handleDeadReplicas()" << endl;
    auto it = replicas.begin();
    auto next = it;
    while (it != replicas.end()) {
      ++next;
      if (currentTime >= it->second.lastPingTime + deadPings) {
        removeReplica(it);
      }
      it = next;
    }
  }

  void updateReplica(const string& name, int viewNum) {
    // cerr << "updateReplica(" << name << ", " << viewNum << ")" << endl;
    replicas[name].lastPingTime = currentTime;
    replicas[name].viewNum = viewNum;

    if (name == currentViewInfo.primary) {
      if (viewNum == 0) {
        removePrimary();
      }
    }
  }

  void removeReplica(unordered_map<string, ReplicaInfo>::iterator it) {
    cerr << "removeReplica(" << it->first << ")" << endl;
    string name = it->first;
    if (currentViewInfo.primary == name) {
      removePrimary();
    }
    if (currentViewInfo.backup == name) {
      removeBackup();
    }
    replicas.erase(it);
  }

  void removePrimary() {
    currentViewInfo.primary = "";
  }

  void removeBackup() {
    currentViewInfo.backup = "";
  }

  void restore() {
    restorePrimary();
    restoreBackup();
  }

  bool restorePrimary() {
    // cerr << "restorePrimary()" << endl;
    if (currentViewInfo.primary != "") {
      return false;
    }
    if (currentViewInfo.backup != "") {
      cerr << "restorePrimary(): " 
            << "swap primary(" << currentViewInfo.primary << ") " 
            << "and backup(" << currentViewInfo.backup << ")" << endl;
      swap(currentViewInfo.primary, currentViewInfo.backup);
      return true;
    } else {
      for (const auto& it : replicas) {
        currentViewInfo.primary = it.first;
        cerr << "restorePrimary(): " << "set new primary " << it.first << endl;
        return true;
      }
    }
    return false;
  }

  bool restoreBackup() {
    // cerr << "restoreBackup()" << endl;
    if (currentViewInfo.backup != "") {
      return false;
    }
    for (const auto& it : replicas) {
      if (it.first != currentViewInfo.primary) {
        currentViewInfo.backup = it.first;
        return true;
      }
    }
    return false;
  }

  int currentTime;
  ViewInfo currentViewInfo;
  unordered_map<string, ReplicaInfo> replicas;

  int deadPings;
};

int main(int argc, char **argv) {
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
  boost::shared_ptr<CoordinatorHandler> handler(new CoordinatorHandler());
  boost::shared_ptr<TProcessor> processor(new CoordinatorProcessor(handler));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(9090));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

  /**
  * Or you could do one of these

  boost::shared_ptr<ThreadManager> threadManager =
  ThreadManager::newSimpleThreadManager(workerCount);
  boost::shared_ptr<PosixThreadFactory> threadFactory =
  boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
  threadManager->threadFactory(threadFactory);
  threadManager->start();
  TThreadPoolServer server(processor,
  serverTransport,
  transportFactory,
  protocolFactory,
  threadManager);

  TThreadedServer server(processor,
  serverTransport,
  transportFactory,
  protocolFactory);
  */

  cout << "Starting the server..." << endl;
  server.serve();
  cout << "Done." << endl;
  return 0;
}
