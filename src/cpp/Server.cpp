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

#include "../gen-cpp/Coordinator.h"
#include "../gen-cpp/ReplicatedStorage_constants.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace logging;

using namespace mipt::distsys::storage;

class CoordinatorHandler : public CoordinatorIf {
public:
  CoordinatorHandler() {
    deadPings = g_ReplicatedStorage_constants.DEAD_PINGS;
    currentTime = 0;
    currentViewInfo.view = 0;
    currentViewInfo.primary = "";
    currentViewInfo.backup = "";
    Log::info("Running server [deadPings: ", deadPings, "]");
  }

  void ping(ViewInfo& viewInfo, const int32_t viewNum, const string& name) {
    Log::trace("ping(", viewNum, ", ", name, ")");
    bool viewChanged = false;
    viewChanged |= updateReplica(name, viewNum);
    viewChanged |= maintainReplication();
    if (viewChanged) {
      ++currentViewInfo.view;
    }
    viewInfo = currentViewInfo;
  }

  void primary(string& result) {
    Log::trace("primary(): ", currentViewInfo.primary);
    result = currentViewInfo.primary;
  }

  int tick() {
    ++currentTime;
    Log::trace("tick(): ", currentTime);
    bool viewChanged = false;
    viewChanged |= handleDeadReplicas();
    viewChanged |= maintainReplication();
    if (viewChanged) {
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

  bool handleDeadReplicas() {
    Log::debug("handleDeadReplicas()");
    bool viewChanged = false;
    auto it = replicas.begin();
    auto next = it;
    while (it != replicas.end()) {
      ++next;
      if (currentTime >= it->second.lastPingTime + deadPings) {
        viewChanged |= removeReplica(it);
      }
      it = next;
    }
    return viewChanged;
  }

  bool updateReplica(const string& name, int viewNum) {
    Log::debug("updateReplica(", name, ", ", viewNum, ")");
    replicas[name].lastPingTime = currentTime;
    replicas[name].viewNum = viewNum;

    if (name == currentViewInfo.primary) {
      if (viewNum == 0) {
        removePrimary();
        return true;
      }
    }
    return false;
  }

  bool removeReplica(unordered_map<string, ReplicaInfo>::iterator it) {
    Log::debug("removeReplica(", it->first, ")");
    string name = it->first;
    if (currentViewInfo.primary == name) {
      if (it->second.viewNum != currentViewInfo.view) {
        Log::warn("removeReplica(): ", "Can't remove primary, backup is not ready");
        return false;
      }
      removePrimary();
      return true;
    }
    if (currentViewInfo.backup == name) {
      removeBackup();
      return true;
    }
    replicas.erase(it);
    return false;
  }

  void removePrimary() {
    Log::trace("removePrimary(): ", currentViewInfo.primary);
    currentViewInfo.primary = "";
  }

  void removeBackup() {
    Log::trace("removeBackup(): ", currentViewInfo.backup);
    currentViewInfo.backup = "";
  }

  bool maintainReplication() {
    bool viewChanged = false;
    viewChanged |= restorePrimary();
    viewChanged |= restoreBackup();
    return viewChanged;
  }

  bool restorePrimary() {
    Log::debug("restorePrimary()");
    if (currentViewInfo.primary != "") {
      return false;
    }
    if (currentViewInfo.backup != "") {
      Log::trace("restorePrimary(): ",
                "swap primary(", currentViewInfo.primary, ") ",
                "and backup(", currentViewInfo.backup, ")");
      swap(currentViewInfo.primary, currentViewInfo.backup);
      return true;
    } else {
      for (const auto& it : replicas) {
        currentViewInfo.primary = it.first;
        Log::trace("restorePrimary(): ", "set new primary ", it.first);
        return true;
      }
    }
    return false;
  }

  bool restoreBackup() {
    Log::debug("restoreBackup()");
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
  using boost::shared_ptr;
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
  shared_ptr<CoordinatorHandler> handler(new CoordinatorHandler());
  shared_ptr<TProcessor> processor(new CoordinatorProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(9090));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

  cout << "Starting the server..." << endl;
  server.serve();
  cout << "Done." << endl;
  return 0;
}
