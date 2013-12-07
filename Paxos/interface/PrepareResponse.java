import java.io.Serializable; 

public class PrepareResponse implements Serializable{

    PrepareResponse(boolean previousAccept, ProposalNumber proposalNumber, int value){
        _previousAccept = false;
        _proposalNumber = proposalNumber;
        _value = value;
    }

    PrepareResponse()
    {
        _previousAccept = false;
        _proposalNumber = new ProposalNumber();
        _value = 0;
    }
  
    public boolean _previousAccept; 
    public ProposalNumber _proposalNumber;
    public int _value; 
}

