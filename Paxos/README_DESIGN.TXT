DESIGN DOCUMENT FOR PROBLEMSET3
IMPLEMENTING THE PAXOS SYNOD ALGOTHRYM

I. DESIGN DECISIONS

A. Language

The server was implemented in java, with the build tasks being handled by Ant.

B. RPC Mechanism

The RPC mechanism chosen was Java RMI or Remote Method Invocation 

C. External Interface

The external interface is captured in paxos.java in the interface sub-directory.  It consists of two methods.  A propose method that takes the value to propose. And a learn method, that returns the value learned from the system.   

public interface Paxos extends Remote {
    void propose(int value) throws RemoteException;
    int learn() throws RemoteException;
}

D. Internal Interface

The internal interface is captured in PaxosInternal.java.  It consists of four methods.  The first two are the methods of the Paxos Synod algorithm.  The last two methods help to propagate the value that is leered.  There is both a "push" and a "pull" method for learning the value that was chosen.  notifyLearners pushes the value to other learners in the system.  internalLearn can be called on a single server in the cluster to discover what value (if any) it has learned. 

public interface PaxosInternal extends Remote {
    PrepareResponse prepare(ProposalNumber propNum) throws RemoteException;
    boolean accept(ProposalNumber propNum, int value) throws RemoteException;
    void notifyLearners(ProposalNumber propNum, int value) throws RemoteException;
    int internalLearn() throws RemoteException;
}

II. INTERESTING ASPECTS

There were two aspects of the implementation that I found particularly interesting.  The first was the idea of "rounds" wasn't well covered in lecture or the slides.  While there was much detail in the materials about the content of each round, there wasn't much detail about how and when to string them together.  For my implementation, I string together the rounds in the Propose external interface call.

The second aspect of the implementation was the idea that servers simply "don't respond" when not given appropriate input.  Of course, that is an interesting idea, but how to implement "not responding".  As a systems programmer the typical "no response" is to return an error code. However, in java there seems to be some bias against error codes and side-effects.  And so in the cases where the specification required an action to "not respond" I interpreted that to mean "throws an exception".  And so that should explain the large number of exception handlers used liberally thought the code. 

III. RUNNING THE CLIENT AND SERVER

A. Building The System

The system builds using ant.  There are several targets defined, but only two are likely to be interesting.  

ant clean
ant all

They do what you would expect.  Ant clean, cleans up all previous build outputs. Ant all builds all targets. 

B. Rmi Registration

The Clients and Servers in the system depend on the java RMI infrastructure.  So you must run rmiregistry before starting either the clients or the servers.  Through experimentation I found that you must have the all the java classes in the directory where you launch rmiregistry from.  This is the classes directory (created during build).  

D. Starting The Server

The server has a very long command line.  As part of the startup it needs to know its own name the port for the local rmiregistry as well as the host and name of the other servers in the cluster. So the arguments are:

java -classpath <classpath> Server <Server Name> <Host for Server2> <Server2 Name> <host for Server3> <server3 name> <port for rmiregistry>

And an example of a working command line for starting all three servers in the cluster:

java -classpath classes Server Paxos1 localhost Paxos2 localhost Paxos3 8123
java -classpath classes Server Paxos2 localhost Paxos1 localhost Paxos3 8123
java -classpath classes Server Paxos3 localhost Paxos1 localhost Paxos2 8123


E. External Interface

The external interface of the system was defined in the assignment. It consists of two methods: propose and learn. The interface as described in Java is as follows:

public interface Paxos extends Remote {
    void propose(int value) throws RemoteException;
    int learn() throws RemoteException;
}


   a. void propose(int value) throws RemoteException;

The propose method takes a single argument, the value being proposed. If a quorum of servers is not available, then the method throws an exception. 

   b. int learn() throws RemoteException;

The learn method takes no arguments and returns a single value, the value learned from the system. If no value has been learned, then it throws an exception. 


F. Internal Interfaces

The internal interface consists of four methods. Prepare and accept are the two methods of the Paxos synod algorithm and the final two methods are "push" and "pull" notifications for values learned. Here's the Java definition of the internal interface. 

public interface PaxosInternal extends Remote {
    PrepareResponse prepare(ProposalNumber propNum) throws RemoteException;
    boolean accept(ProposalNumber propNum, int value) throws RemoteException;
    void notifyLearners(ProposalNumber propNum, int value) throws RemoteException;
    int internalLearn() throws RemoteException;
}


   a. PrepareResponse prepare(ProposalNumber propNum) throws RemoteException;

This is the prepare message from the Paxos Synod algorithm.  A proposal number is a tuple consisting of an integer and the name of the server that is doing the proposing; for example (10, "Paxos1").  The prepare response consists of a boolean, a ProposalNumber, and a value.  The boolean tells you if the other two values are valid.  If true, the proposal number is the proposal number of the last value accepted, and the value is the value that was accepted. 

   b. boolean accept(ProposalNumber propNum, int value) throws RemoteException;

This is the accept method from the paxos synod algorithm.  It returns true if the proposal was accepted, and it throws an exception in all other cases. 

   c. void notifyLearners(ProposalNumber propNum, int value) throws RemoteException;

The notify learners method lets each node in the cluster know that a value was chosen, what the value was, and what Proposal Number it was accepted on. 

   d. int internalLearn() throws RemoteException;

The internalLearn method is a method that can be used to determine if a value was chosen by the system.  It returns the value that was learned. 

G. Running the client

The client application can access both the internal and external interfaces 

   a. Internal Interfaces

The client can be used to access the external interfaces.  Here are examples of the client calling on the external interfaces. The general form of the command is below

java -classpath <classes> Client <host for RMI registy> <name of RMI object> <Port for RMI registry> <command> <command arguments>

This command line uses the Paxos1 node to propose the value 100

   java -classpath classes Client localhost Paxos1 8123 propose 1000

This command line uses the Paxos1 node to learn what value was learned.

   java -classpath classes Client localhost Paxos1 8123 learn

   b. External Interfaces

The client can also be used to access both the propose and accept methods on the internal interfaces. This feature was added for testing purposes.  It allows the client to put the node into any state achievable through a series of calls to the propose and accept methods of the internal interfaces

  java -classpath classes Client localhost Paxos3 8123 prepare 3 Paxos2

The example above calls prepare method on the paxos3 node with the proposal number (3, "Paxos2")

java -classpath classes Client localhost Paxos3 8123 accept 3 Paxos2 987

The example above calls the accept method on the Paxos3 node with the proposal number (3, "Paxos2") and a value of 987.



