
#include <vector>
#include <list>
#include "Unit.h"
#include "UPCluster.h"
using std::vector;
using std::list;

namespace SparCraft
{
	class UPGMA
	{
	public:
		UPGMA(const vector<Unit> in, const double hpMultiplier, const double distMultiplier);
		~UPGMA();

		
		void getClusters(int clusters,vector<vector<Unit>> clusteredUnits);

	private:
		
		int K;			// The number of clusters created so far
		vector<UPCluster> cluster;		// The nodes (clusters) of the resulting tree
		vector<Unit> input;
		double hpMultiplier;
		double distMultiplier;

		double d(const int i, const int j);

		const UPCluster getRoot();

		void findAndJoin();

		void join(int i, int j);

		list<UPCluster> cutTree(UPCluster c, int clusterLimit);

		const list<int> getLeafs(const UPCluster c);

		void createDistanceMatrix(double** ds, vector<Unit> strings);
		vector<double> createDistanceVector(int i, vector<Unit> units);
		const double getDistance(const Unit a,const  Unit b);
	};


	UPGMA::~UPGMA()
	{
	}


	// UPGMA clusters or trees, built by the UPGMA algorithm
	
}
