import java.io.Serializable; 

public class ProposalNumber implements Serializable, Comparable<ProposalNumber> {

    ProposalNumber(int number, String proposer){
        _number = number;
        _proposer = proposer;
    }

    ProposalNumber(){
        _number = 0;
        _proposer = "a";
    }

    public boolean equals(Object obj){
        if(this == obj) {
            return true; 
        }

        if (obj instanceof ProposalNumber) {
            ProposalNumber propNum = (ProposalNumber)obj;
            return ((this._number == propNum._number) && (this._proposer.equals(propNum._proposer)));
            
        }
        
        return false;
    }

    public int compareTo(ProposalNumber proposalNumber){
        // 0 -- equal
        // -1 -- less than
        // 1 -- greater than

        if (this._number == proposalNumber._number) {
            return this._proposer.compareTo(proposalNumber._proposer);
        }

        return this._number - proposalNumber._number;
    }

    public int _number;
    public String _proposer;
}
