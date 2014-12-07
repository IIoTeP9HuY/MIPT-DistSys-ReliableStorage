package mipt.distsys.storage.proxy;

import java.util.*;

public class TestFailedException extends Exception
{
    TestFailedException(String message)
    {
        super(message);
    }
}
