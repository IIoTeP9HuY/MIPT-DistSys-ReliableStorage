package mipt.distsys.storage.proxy;

import java.rmi.*;
import java.util.*;

import mipt.distsys.storage.ViewInfo;

public interface CoordinatorInterface extends Remote
{
    public ViewInfo ping(int viewNum, String name) throws RemoteException;
    public String primary() throws RemoteException;
}
