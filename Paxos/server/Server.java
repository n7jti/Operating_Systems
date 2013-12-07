import java.rmi.registry.Registry;
import java.rmi.registry.LocateRegistry;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.*;
import java.io.*;
	
public class Server implements Paxos, PaxosInternal {
	
    public Server() {
        _lock = new java.util.concurrent.locks.ReentrantLock();
        try{
            ReadState();
        }catch (java.io.FileNotFoundException e){
             _previousAccept = false;
            _lastAcceptedValue = 0;
            _lastAcceptedProposalNumber = new ProposalNumber();
            _valueLearned = false;
            _learnedValue = 0;    
            _lastPrepareProposalNumber = new ProposalNumber();
            _learnedProposalNumber = new ProposalNumber();
        }
    }

    public void propose(int value) throws RemoteException{
        // Get a set of participants
        Collection<PaxosInternal> acceptors = new LinkedHashSet<PaxosInternal>();

        // set the value we will propose
        _value = value;

        while(true) {     
            try{
                acceptors.clear();
                acceptors.add(this);
                //get pointer to the 1st other server
                addStub(acceptors, _host1, _service1);
                addStub(acceptors, _host2, _service2); 
                
                if (acceptors.size() < 2) {
                    throw new RemoteException("A majority of serviers is unreachable!");
                }

                PrepareResults prepareResults = proposalPrepare(acceptors);

                if(prepareResults != PrepareResults.VALUE_CHOSEN) {
                    proposalAccept(acceptors);
                }

                break;
            }catch (RemoteException e){
            }
        }

        proposalNotify(acceptors);
    }

    private PrepareResults proposalPrepare(Collection<PaxosInternal> acceptors) throws java.rmi.RemoteException{
        Collection<PrepareResponse> prepareResponses = new LinkedHashSet<PrepareResponse>();
       

        while(prepareResponses.size() < 2) {
            prepareResponses.clear();
            _proposalCount++;
            System.out.println(String.format("Round #%d started by %s",_proposalCount, _service));
            ProposalNumber proposalNumber = new ProposalNumber(_proposalCount, _service);

            for(PaxosInternal pi : acceptors) {    
                try{
                    PrepareResponse singleResponse;
                    singleResponse = pi.prepare(proposalNumber);
                    prepareResponses.add(singleResponse);
                }catch (java.rmi.RemoteException e){
                    ;
                }
            }

        }

        return analyzePrepare(prepareResponses);
    }

    private enum PrepareResults{
        NO_VALUES_ACCEPTED,
        NO_VALUES_CHOSEN,
        VALUE_CHOSEN

    }

    private PrepareResults analyzePrepare(Collection<PrepareResponse> prepareResponses) throws java.rmi.RemoteException{
        // There are three possible outcomes
        // 1. No values were previously accepted
        // 2. A value has been previously accepted, but not yet chosen
        //      Check the count on the maximum response.  if it is not a majority then we're in this state
        //      this is detected when a majorit of responses have previously accepted the same value.
        // 3. A value has been previously chosen
        //      this happens when the maximum response count is a majority

        PrepareResults result = PrepareResults.NO_VALUES_ACCEPTED; // no values previously accepted

        PrepareResponse maxPrevAccept = new PrepareResponse();
        int countMax = 0;

        for(PrepareResponse resp : prepareResponses) {

            if(resp._previousAccept) {
                int compare = maxPrevAccept._proposalNumber.compareTo(resp._proposalNumber);
                if (compare == 0) {
                    // another instance of the max
                    countMax++;
                }
                else if (compare < 0) {
                    // new max
                    countMax = 0;
                    maxPrevAccept = resp;
                    _value = resp._value;
                }
            }
        }

        // Now. determine state
        if (maxPrevAccept._previousAccept == true) {
            if (countMax < 2) { // 2 is a majority
                result = PrepareResults.NO_VALUES_CHOSEN; // value not yet chosen
            }
            else { // 2 is a majority
                result = PrepareResults.VALUE_CHOSEN; // a value was chosen
                // we just learned the result!
                this._learnedProposalNumber = maxPrevAccept._proposalNumber;
                this._learnedValue = maxPrevAccept._value;
                this._valueLearned = true;

                try{
                    WriteState();
                }catch (java.io.IOException e){
                    throw new java.rmi.RemoteException("Failed to write state.", e);
                }
            }
        }

        return result;
    }

    private void proposalAccept(Collection<PaxosInternal> acceptors) throws java.rmi.RemoteException{
        int acceptResponses = 0;

        ProposalNumber proposalNumber = new ProposalNumber(this._proposalCount, this._service);
        for(PaxosInternal pi : acceptors) {    
            try{
                boolean acceptResponse;
                acceptResponse = pi.accept(proposalNumber, _value);
                if(acceptResponse) {
                    acceptResponses++;
                }
            }catch (java.rmi.RemoteException e){
                 System.out.println("A server did not accept");
            }
        }

        if (acceptResponses < 2) {
            // a majority of the synod did not accept the proposal
            throw new java.rmi.RemoteException("Less than a majority of participants accepted");
        }

        // We just learned the result
        this._learnedProposalNumber = proposalNumber;
        this._learnedValue = _value;
        this._valueLearned = true;

        try{
            WriteState();
        }catch (java.io.IOException e){
            throw new java.rmi.RemoteException("Failed to write state.", e);
        }

        return;
    }

    private void proposalNotify(Collection<PaxosInternal> acceptors)
    {
         for(PaxosInternal pi : acceptors) {    
            try{
                pi.notifyLearners (this._learnedProposalNumber, this._learnedValue);
            }catch (java.rmi.RemoteException e){
                 System.err.println("Server Exception: " + e.toString());
                 e.printStackTrace();
            }
        }

    }

    private void addStub(Collection<PaxosInternal> acceptors, String host, String service){
        try{
            Registry registry = LocateRegistry.getRegistry(host, _rmiPort);
            PaxosInternal stub = (PaxosInternal) registry.lookup(service);
            acceptors.add(stub);
        }catch (java.rmi.RemoteException e){
            System.out.println("Remote Server Exception: " + e.toString());
	    e.printStackTrace();
        }catch (java.rmi.NotBoundException e) {
            System.out.println("Server not found: " + service);
        }

    }

    public int learn() throws RemoteException{
        _lock.lock();
        try{
            if(_valueLearned == false) {
                // check with the other servers
                // Get a set of participants
                Collection<PaxosInternal> acceptors = new LinkedHashSet<PaxosInternal>();

                //get pointer to the 1st other server
                addStub(acceptors, _host1, _service1);
                addStub(acceptors, _host2, _service2);

                for(PaxosInternal pi : acceptors) {    
                    try{
                        _learnedValue = pi.internalLearn();
                        _valueLearned = true;

                        // Just learned the value;
                        WriteState();
                        break;
                    }catch (java.rmi.RemoteException e){
                        // Exceptions here are expected.
                    }catch (java.io.IOException e){
                        throw new java.rmi.RemoteException("Failed to write state.", e);
                    }
                }

                if (_valueLearned == false) {
                    throw new java.rmi.RemoteException("No Value Learned");
                }
            }
        }finally{
            _lock.unlock();
        }
        return _learnedValue;
    }

    public int internalLearn() throws java.rmi.RemoteException{
        if (!_valueLearned) {
            throw new java.rmi.RemoteException("No Value Learned");
        }

        return _learnedValue;
    }

    public PrepareResponse prepare(ProposalNumber propNum) throws java.rmi.RemoteException{
        PrepareResponse prepareResponse = new PrepareResponse();
        _lock.lock();
        try{
            if (propNum.compareTo(_lastPrepareProposalNumber) > 0) {
                _lastPrepareProposalNumber = propNum;
                prepareResponse._previousAccept = _previousAccept;
                if(_previousAccept) {
                    prepareResponse._proposalNumber = propNum;
                    prepareResponse._value = _lastAcceptedValue;
                }
            }
            else{
                System.out.println("Prepare rejected by " + _service + " proposal number too low.");
                throw new java.rmi.RemoteException("Stale Proposal Recieved.");
            }

            try{
                WriteState();
            }catch (java.io.IOException e){
                throw new java.rmi.RemoteException("Failed to write state.", e);
            }
        }finally{
            _lock.unlock();
        }
        System.out.println("Prepare accepted by " + _service);
        return prepareResponse;
    }


    public boolean accept(ProposalNumber propNum, int value) throws java.rmi.RemoteException{
        _lock.lock();
        try{

            // I assuming that the server can only accept a proposal that matches the most recent perpare. 
            if (propNum.compareTo(_lastPrepareProposalNumber) != 0) {
                System.out.println("Accept rejected by " + _service + " proposal number does not match last prepare.");
                throw new java.rmi.RemoteException("Proposal number does not match last prepare");
            }

            _previousAccept = true;
            _lastAcceptedProposalNumber = propNum;
            _lastAcceptedValue = value;

            try{
                WriteState();
            }catch (java.io.IOException e){
                throw new java.rmi.RemoteException("Failed to write state.", e);
            }

        }finally{
            _lock.unlock();
        }
        System.out.println("Value accepted by " + _service + ":" + _lastAcceptedValue);
        return true;
    }

    public void notifyLearners(ProposalNumber propNum, int value) throws RemoteException{
        _lock.lock();
        try{
            _valueLearned = true;
            _learnedValue = value;

            try{
                WriteState();
            }catch (java.io.IOException e){
                throw new java.rmi.RemoteException("Failed to write state.", e);
            }
        }finally{
            _lock.unlock();
        }
    }

    public static void usage(){
        System.err.println("Args: <name> <host1> <name 1> <host 2> <name 2> <rmi registry port");
    }

    public static void main(String args[]) {
        if (args.length < 6) {
            usage();
            throw new java.lang.IllegalArgumentException("Too Few Arguments");
        }

        _service = args[0];
        _host1 = args[1];
        _service1 = args[2];
        _host2 = args[3];
        _service2 = args[4];
        _rmiPort = Integer.parseInt(args[5]);
	
	try {
	    Server obj = new Server();
	    Paxos paxosStub = (Paxos) UnicastRemoteObject.exportObject(obj, 0);

	    // Bind the remote object's stub in the registry
	    Registry registry = LocateRegistry.getRegistry(_rmiPort);

            try{
                registry.bind(_service, paxosStub);
            }catch (java.rmi.AlreadyBoundException abe){
                registry.rebind(_service, paxosStub);
            }
             
	} catch (Exception e) {
	    System.err.println("Server exception: " + e.toString());
	    e.printStackTrace();
	}
    }

    private void WriteState() throws java.io.IOException{
        PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(_service + ".txt")));

        // acceptor state
        out.printf("%s\n", _previousAccept ? "True" : "False");
        out.printf("%d\n", _lastAcceptedValue);
        out.printf("%d\n", _lastAcceptedProposalNumber._number);
        out.printf("%s\n", _lastAcceptedProposalNumber._proposer);
        out.printf("%d\n", _lastPrepareProposalNumber._number);
        out.printf("%s\n", _lastPrepareProposalNumber._proposer);
        

        // learner state
        out.printf("%s\n", _valueLearned ? "True" : "False");
        out.printf("%d\n", _lastAcceptedValue);
        out.printf("%d\n", _learnedProposalNumber._number);
        out.printf("%s\n", _learnedProposalNumber._proposer);

        // proposer state
        out.printf("%d\n", _proposalCount);
        out.printf("%d\n", _value);
        out.flush();

    }

    private void ReadState() throws java.io.FileNotFoundException {
        Scanner in = new Scanner(new BufferedReader(new FileReader(_service + ".txt")));
        
        // acceptor state
        _previousAccept = in.nextBoolean();
        _lastAcceptedValue = in.nextInt();

        int number = in.nextInt();
        String proposer = in.next();
        _lastAcceptedProposalNumber = new ProposalNumber(number, proposer);

        number = in.nextInt();
        proposer = in.next();
        _lastPrepareProposalNumber = new ProposalNumber(number, proposer);


        // lerner state
        _valueLearned = in.nextBoolean();
        _learnedValue = in.nextInt();

        number = in.nextInt();
        proposer = in.next();
        _learnedProposalNumber = new ProposalNumber(number, proposer);

        // proposer state
         _proposalCount = in.nextInt();
        _value = in.nextInt();
    }

    // acceptor state
    private boolean _previousAccept;
    private int _lastAcceptedValue;
    private ProposalNumber _lastAcceptedProposalNumber;
    private ProposalNumber _lastPrepareProposalNumber;

    // learner state
    private boolean _valueLearned;
    private int _learnedValue;
    private ProposalNumber _learnedProposalNumber;

    // proposer state
    private int _proposalCount;
    private int _value;

    // lock
    private final java.util.concurrent.locks.ReentrantLock _lock;  

    //Command line arguments
    static private String _service;
    static private String _host1;
    static private String _service1;
    static private String _host2;
    static private String _service2;
    static private int _rmiPort;
}

