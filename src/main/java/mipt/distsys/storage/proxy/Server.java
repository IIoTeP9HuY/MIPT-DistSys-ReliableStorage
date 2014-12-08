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

public class Server extends UnicastRemoteObject implements ServerInterface
{
    Process serverProcess;
    TTransport transport;
    TProtocol protocol;
    mipt.distsys.storage.Server.Client client;

    public Server(String serverName, String coordName) throws Exception {
        HostPort server = new HostPort(serverName);
        HostPort coord = new HostPort(coordName);
        try {
            runServer(server.port, server.host);
            transport = new TSocket(server.host, server.port);
            transport.open();

            protocol = new TBinaryProtocol(transport);
            client = new mipt.distsys.storage.Server.Client(protocol);
            client.setCoordinator(coordName);
        } catch (TException x) {
            x.printStackTrace();
            throw new RemoteException();
        }
    }

    protected void runServer(int port, String serverName) throws Exception {
        ProcessBuilder pb = new ProcessBuilder("build/server", String.valueOf(port), serverName);
        pb.redirectOutput(ProcessBuilder.Redirect.INHERIT);
        pb.redirectError(ProcessBuilder.Redirect.INHERIT);
        serverProcess = pb.start();
        Thread.sleep(300);
    }

    @Override
    protected void finalize() {
        transport.close();
        serverProcess.destroy();
    }

    public void put(String key, String value) throws RemoteException {
        try {
            client.put(key, value);
        } catch (mipt.distsys.storage.IncorrectOperationException e) {
            throw new IncorrectOperationException(e.why);
        } catch (TException x) {
            x.printStackTrace();
            throw new RemoteException();
        }
    }

    public void putBackup(String key, String value) throws RemoteException {
        try {
            client.putBackup(key, value);
        } catch (mipt.distsys.storage.IncorrectOperationException e) {
            throw new IncorrectOperationException(e.why);
        } catch (TException x) {
            x.printStackTrace();
            throw new RemoteException();
        }
    }

    public String get(String key) throws RemoteException {
        try {
            return client.get(key);
        } catch (mipt.distsys.storage.IncorrectOperationException e) {
            throw new IncorrectOperationException(e.why);
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
