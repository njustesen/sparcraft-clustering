#include <vector>
using std::vector;

namespace SparCraft{


class UPCluster {

	public:
		//UPCluster::UPCluster();
		
		//UPCluster::UPCluster(int lab, vector<double> dmat);
		//UPCluster::UPCluster(int lab, UPCluster* left, UPCluster* right, double height, vector<double> dmat);


		int lab;			// Cluster identifier
		int card;			// The number of sequences in the cluster
		double height;		// The height of the node
		UPCluster *left, *right;	// Left and right children, or null
		vector<double> dmat;		// Distances to lower-numbered nodes, or null


		UPCluster(UPCluster* cluster){
			int lab=cluster->lab;			// Cluster identifier
			int card=cluster->card;			// The number of sequences in the cluster
			double height=cluster->height;		// The height of the node
			UPCluster *left=cluster->left, *right=cluster->right;	// Left and right children, or null
			vector<double> dmat=cluster->dmat;		// Distances to lower-numbered nodes, or null
		}

		UPCluster(int _lab, vector<double> _dmat) {	// Leaves = single sequences
			lab = _lab + 1; 
    		card = 1;
    		dmat = _dmat;
		}

		UPCluster(int _lab, UPCluster* _left, UPCluster* _right, double _height, vector<double> _dmat)
			: lab(_lab+1)
			, left(_left)
			, right(_right)
			, card(_left->card + _right->card)
			, height(_height)
		{ 
			dmat = _dmat;
		}
		UPCluster *       getLeft()                 const           { return left; }
		UPCluster *       getRight()                const           { return right; }
		bool live(){ return dmat == vector<double>(-1); }

		void kill() { dmat = vector<double>(-1); }

		/*public void print() 
		{ print(0); }

  		void print(int n) {
  			if (right != null)
  				right.print(n + 6);
    		indent(n); 
    		System.out.println("[" + lab + "] (" + (int)(100*height)/100.0 + ")"); 
    		if (left != null)
    			left.print(n + 6);
  		}

  		void indent(int n) {
  			for (int i=0; i<n; i++)
  				System.out.print(" ");
  		}*/
		bool UPCluster::operator==(const UPCluster &other) const {
			//...  // Compare the values, and return a bool result.
			return this->lab==other.lab;			// Cluster identifier
			/*this->card==;			// The number of sequences in the cluster
			this->height;		// The height of the node
			this->left;
			this->right;	// Left and right children, or null
			this->dmat;*/
		  }
		bool UPCluster::operator!=(const UPCluster &other) const {
			return !(*this == other);
		  }
	};
}