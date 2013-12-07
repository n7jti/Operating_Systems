
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class Client {

    private Client() {}

    public static void usage() {
        System.err.println("Args: <host> <name> <rmi registry port>  <comand> <comand args>");
    }

    public static void main(String[] args) {
	String host = args[0];
        String service = args[1];
        int rmiPort = Integer.parseInt(args[2]);
        String command = args[3];

        try {
            Registry registry = LocateRegistry.getRegistry(host, rmiPort);
	    Paxos stub = (Paxos) registry.lookup(service);
       

            if (command.equals("propose")) {
                stub.propose(Integer.parseInt(args[4]));
                System.out.println("Proposed: " + args[4]);
            } else if (command.equals("learn")) {
                int value = stub.learn();
                System.out.println(value + " learned!");
            } else if(command.equals("prepare")) {
                PaxosInternal pi = (PaxosInternal)stub;
                PrepareResponse resp = pi.prepare(new ProposalNumber(Integer.parseInt(args[4]),args[5]));
                String output = String.format("Prepare Response: %b %d %s %d", resp._previousAccept, resp._proposalNumber._number, resp._proposalNumber._proposer, resp._value);
                System.out.println(output);
            } else if (command.equals("accept")) {
                ProposalNumber proposalNumber = new ProposalNumber(Integer.parseInt(args[4]),args[5]);
                int value = Integer.parseInt(args[6]);
                PaxosInternal pi = (PaxosInternal)stub;
                boolean accepted = pi.accept(proposalNumber, value);
                String output = String.format("Accept: %b %d %s %d", accepted, proposalNumber._number, proposalNumber._proposer, value);
                System.out.println(output);
            }
            else {
                throw new java.lang.IllegalArgumentException("Unrecognized Command: " + command);
            }

        } catch (Exception e) {
	    System.err.println("Client exception: " + e.toString());
	    //e.printStackTrace();
	}
	
    }
}
