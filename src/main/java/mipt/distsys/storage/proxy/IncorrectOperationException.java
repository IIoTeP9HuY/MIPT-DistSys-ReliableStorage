package mipt.distsys.storage.proxy;

import java.util.*;

public class IncorrectOperationException extends RuntimeException
{
    IncorrectOperationException(String message)
    {
        super(message);
    }
}
