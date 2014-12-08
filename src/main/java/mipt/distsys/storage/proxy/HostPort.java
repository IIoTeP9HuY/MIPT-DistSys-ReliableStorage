package mipt.distsys.storage.proxy;

import java.io.*;

public class HostPort implements Serializable {
    public String host;
    public int port;

    public HostPort(String host, int port) {
        this.host = host;
        this.port = port;
    }

    public HostPort(String hostPort) {
        String[] hostAndPort = hostPort.split(":");
        this.host = hostAndPort[0];
        this.port = Integer.parseInt(hostAndPort[1]);
    }

    @Override
    public boolean equals(Object o) {
        HostPort other = (HostPort) o;
        return host.equals(other.host) && port == other.port;
    }

    @Override
    public int hashCode() {
        return host.hashCode();
    }

    @Override
    public String toString() {
        return host + ":" + port;
    }
}
