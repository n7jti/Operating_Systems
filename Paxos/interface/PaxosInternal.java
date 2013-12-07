import java.rmi.Remote;
import java.rmi.RemoteException;

public interface PaxosInternal extends Remote {
    PrepareResponse prepare(ProposalNumber propNum) throws RemoteException;
    boolean accept(ProposalNumber propNum, int value) throws RemoteException;
    void notifyLearners(ProposalNumber propNum, int value) throws RemoteException;
    int internalLearn() throws RemoteException;
}
