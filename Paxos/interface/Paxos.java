import java.rmi.Remote;
import java.rmi.RemoteException;

public interface Paxos extends Remote {
    void propose(int value) throws RemoteException;
    int learn() throws RemoteException;
}
