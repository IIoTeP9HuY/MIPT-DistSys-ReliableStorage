package mipt.distsys.storage.proxy;

import java.rmi.*;
import java.util.*;

public interface ServerInterface extends Remote
{
    public void put(String key, String value) throws RemoteException;
    public void putBackup(String key, String value) throws RemoteException;
    public String get(String key) throws RemoteException;
}
