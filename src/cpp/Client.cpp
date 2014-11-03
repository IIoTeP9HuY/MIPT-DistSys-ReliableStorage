#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "../gen-cpp/Coordinator.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace mipt::distsys::storage;

int main(int argc, char** argv) {
  boost::shared_ptr<TTransport> socket(new TSocket("0.0.0.0", 9090));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  CoordinatorClient client(protocol);

  try {
    transport->open();

    ViewInfo viewInfo;
    client.ping(viewInfo, 1, "localhost");
    cout << "ping()" << endl;

    string primary;
    client.primary(primary);
    cout << "primary() = " << primary << endl;

    transport->close();
  } catch (TException& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }

}
