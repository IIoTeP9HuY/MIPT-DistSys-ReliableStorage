package mipt.distsys.storage.proxy;

import java.rmi.*;
import java.rmi.server.*;
import java.util.*;

import org.apache.thrift.TException;
import org.apache.thrift.transport.TSSLTransportFactory;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TSSLTransportFactory.TSSLTransportParameters;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;

import mipt.distsys.storage.ViewInfo;

public class Coordinator extends UnicastRemoteObject implements CoordinatorInterface
{
    public static final int deadPings
        = mipt.distsys.storage.ReplicatedStorageConstants.DEAD_PINGS;
    Process coordinatorProcess;
    TTransport transport;
    TProtocol protocol;
    mipt.distsys.storage.Coordinator.Client client;

    public Coordinator(int port) throws Exception {
        try {
            runCoordinator(port);
            transport = new TSocket("0.0.0.0", port);
            transport.open();

            protocol = new TBinaryProtocol(transport);
            client = new mipt.distsys.storage.Coordinator.Client(protocol);
        } catch (TException x) {
            x.printStackTrace();
            throw new RemoteException();
        }
    }

    protected void runCoordinator(int port) throws Exception {
        ProcessBuilder pb = new ProcessBuilder("build/coordinator", String.valueOf(port));
        pb.redirectOutput(ProcessBuilder.Redirect.INHERIT);
        pb.redirectError(ProcessBuilder.Redirect.INHERIT);
        coordinatorProcess = pb.start();
        Thread.sleep(300);
    }

    @Override
    protected void finalize() {
        transport.close();
        coordinatorProcess.destroy();
    }

    // this method is to be called by server
    public ViewInfo ping(int view, String serverName) throws RemoteException {
        try {
            return client.ping(view, serverName);
        } catch (TException x) {
            x.printStackTrace();
            throw new RemoteException();
        }
    }

    // this method is to be called by client
    public String primary() throws RemoteException {
        try {
            return client.primary();
        } catch (TException x) {
            x.printStackTrace();
            throw new RemoteException();
        }
    }

    // this method is to be called automatically as time goes by
    public int tick() throws RemoteException {
        try {
            return client.tick();
        } catch (TException x) {
            x.printStackTrace();
            throw new RemoteException();
        }
    }
}
