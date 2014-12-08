package mipt.distsys.storage.proxy;

import java.util.*;
import java.rmi.*;

public class TestServer
{
    static void test(boolean condition) throws TestFailedException
    {
        if (!condition) {
            throw new TestFailedException("FAILED");
        }
    }

    public static void main (String[] argv) throws Exception
    {
        if (argv.length != 1) {
            System.out.println("Usage: java " + TestServer.class.getName() + " PORT");
        }

        int coordPort = Integer.parseInt(argv[0]);
        int srv1Port = coordPort + 1;
        int srv2Port = coordPort + 2;

        String coordName = "coordinator1";
        String srv1Name = "server1";
        String srv2Name = "server2";

        Coordinator coordinator = new Coordinator(coordPort);
        // Naming.rebind(coordName, coordinator);

        Server server1 = new Server(srv1Name, coordName, srv1Port, coordPort);
        Server server2 = new Server(srv2Name, coordName, srv2Port, coordPort);
        // Naming.rebind(srv1Name, server1);
        // Naming.rebind(srv2Name, server2);

        int longDelay = Coordinator.deadPings * 2;

        // first primary
        for (int i = 0; i < longDelay; ++i) {
            coordinator.tick();
            server1.tick();
        }
        test(coordinator.primary().equals(srv1Name));
        server1.put("a", "aaa");
        test(server1.get("a").equals("aaa"));

        // first backup
        for (int i = 0; i < longDelay; ++i) {
            coordinator.tick();
            server1.tick();
            server2.tick();
        }
        test(coordinator.primary().equals(srv1Name));
        server1.put("b", "bbb");
        test(server1.get("b").equals("bbb"));

        // primary fails
        java.rmi.server.UnicastRemoteObject.unexportObject(server1, true);
        server1.finalize();
        for (int i = 0; i < longDelay; ++i) {
            coordinator.tick();
            server2.tick();
        }
        test(coordinator.primary().equals(srv2Name));
        test(server2.get("a").equals("aaa"));
        test(server2.get("b").equals("bbb"));

        // ex-primary restarts
        server1 = new Server(srv1Name, coordName, srv1Port, coordPort);
        // Naming.rebind(srv1Name, server1);
        for (int i = 0; i < longDelay; ++i) {
            coordinator.tick();
            server1.tick();
            server2.tick();
        }
        test(coordinator.primary().equals(srv2Name));

        // oh no! network partition
        // client sees server2, but coordinator does not
        for (int i = 0; i < longDelay; ++i) {
            coordinator.tick();
            server1.tick();
        }
        test(coordinator.primary().equals(srv1Name));
        test(server1.get("a").equals("aaa"));
        test(server1.get("b").equals("bbb"));
        try {
            server2.put("c", "ccc");
            throw new TestFailedException("Must throw!");
        } catch(IncorrectOperationException ex) {
            // ok
        }

        System.out.println("Everything ok!");

        java.rmi.server.UnicastRemoteObject.unexportObject(coordinator, true);
        coordinator.finalize();
        java.rmi.server.UnicastRemoteObject.unexportObject(server1, true);
        server1.finalize();
        java.rmi.server.UnicastRemoteObject.unexportObject(server2, true);
        server2.finalize();
    }
}
