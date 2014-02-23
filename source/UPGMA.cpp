/**
 * OIRGINAL CODE FROM http://www.itu.dk/~sestoft/bsa.html, http://www.itu.dk/~sestoft/bsa/Match2.java
 */
#include "UPGMA.h"

using namespace SparCraft;

UPGMA::UPGMA(vector<Unit> in, double ihpMultiplier, double idistMultiplier)
	: hpMultiplier(ihpMultiplier)
	, distMultiplier(idistMultiplier)
{
	//hpMultiplier=ihpMultiplier;
	//distMultiplier=idistMultiplier;
	int z(in.size());
	for (int i(0); i<z;i++){
		if (in[i].isAlive()){ //TODO input[i]!=NULL
			input.push_back(in[i]);
		}
	}
	//double ds [input.size][input.size];
	
	//createDistanceMatrix(ds, input);
	const int N = input.size();
	K = N;
	if (K==0)
		return;
	//cluster.size=2*N-1;
	for (int i(0); i<N; i++){ 

		//cluster[i] = UPCluster(i, ds[i]);
		cluster[i]=UPCluster(i, createDistanceVector(i,input));
	}
	
	    while (K < 2*N-1){
	    	findAndJoin();
	}
}

const UPCluster UPGMA::getRoot()
{ 
	return cluster[K-1]; 
}
	  
double UPGMA::d(int i, int j) 
{ 
	return cluster[std::max (i, j)].dmat[std::min(i, j)]; 
}
		
void UPGMA::findAndJoin() 
{ // Find closest two live clusters and join them
	int mini = -1, minj = -1;
	double mind = std::numeric_limits<double>::max();
	for (int i(0); i<K; i++) {
	    if (cluster[i].live()){
			for (int j=0; j<i; j++) {
				if (cluster[j].live()) {
					double di = d(i, j);
					if (di < mind) {
						mind = di;
						mini = i;
						minj = j;
					}
				}
			}
		}
	}
	join(mini, minj);
}
	
void UPGMA::join(int i, int j) 
{ // Join i and j to form node K
	//  System.out.println("Joining " + (i+1) + " and " + (j+1) + " to form "    + (K+1) + " at height " + (int)(d(i, j) * 50)/100.0);

	vector<double> dmat;
	//dmat.size=K;
	for (int m(0); m<K; m++){
		if (cluster[m].live() && m != i && m != j) {
			dmat[m] = (d(i, m) * cluster[i].card + d(j, m) * cluster[j].card) / (cluster[i].card + cluster[j].card);
		}
	}
	UPCluster* p1;
	p1= & cluster[i];
	UPCluster* p2;
	p2= & cluster[j];
	cluster[K] = UPCluster(K,  p1, p2, d(i, j) / 2, dmat);
	cluster[i].kill(); 
	cluster[j].kill();
	K++;
}

	
void  UPGMA::getClusters(int clusters, vector<vector<Unit>> clusteredUnits){
	if (K==0)
		return;
	list<UPCluster> up = cutTree(getRoot(), clusters);
	//HashMap<Integer, List<Unit>> result = new HashMap<Integer,List<Unit>>();
	for (list<UPCluster>::iterator it=up.begin(); it != up.end(); ++it){
		vector<Unit> tmp;
		list<int> leafs=getLeafs(*it);
		for (list<int>::iterator ut=leafs.begin(); ut != leafs.end(); ++ut){
			tmp.push_back(input[*ut-1]);
		}
			
		clusteredUnits.push_back(tmp);
	}
	
}
	
const list<int> UPGMA::getLeafs(const UPCluster c){
	list<int> ids;
	if (c.left){ //!= NULL){
		list<int> temp=getLeafs(*c.left);
		for (list<int>::iterator it=temp.begin(); it != temp.end(); ++it){
			ids.push_back (*it);
		}
	}
	if (c.right){ // != NULL){
		list<int> temp=getLeafs(*c.right);
		for (list<int>::iterator it=temp.begin(); it != temp.end(); ++it){
			ids.push_back (*it);
		}
	}
	if (c.left/* == NULL*/ && c.right){ // == NULL){
		ids.push_back (c.lab);
	}
	return ids;
}
	
	
list<UPCluster> UPGMA::cutTree(UPCluster c, int clusterLimit)
{
	list<UPCluster> up;
	list<UPCluster> q;
	if (c.card > 0 ){//&& c.left != NULL && c.right != NULL){

		q.push_back(c);
		up.push_back(c);
		int size(up.size());
		while (size<clusterLimit && !(q.empty())){
			UPCluster current =q.front();
			q.pop_front();
			if (current.card > 0 && current.left != NULL && current.right != NULL){
				up.remove(current);
				up.push_back(*current.left);
				up.push_back(*current.right);
				if (current.left->height>current.right->height){
					q.push_back(current.left);
					q.push_back(current.right);
				} else{
					q.push_back(current.right);
					q.push_back(current.left);
				}
			}
			size=up.size();
		}
			
	} else {
		up.push_back(c);
	}
	return up;
}
	
void UPGMA::createDistanceMatrix(double** ds,vector<Unit> strings)
{
	int size(strings.size());	
	if ( size> 0){
		for (int i(1); i <size;i++){
			for (int j(0);j<i;j++){
				//This implementation follows from Algorithms on Strings, Trees and Sequences by Dan Gusfield and Chas Emerick's implementation of the Levenshtein distance algorithm from http://www.merriampark.com/ld.htm					
				ds[i][j] =getDistance(strings[i], strings[j]);
			}
		}
	}
}

vector<double> UPGMA::createDistanceVector(int i, vector<Unit> units)
{
	vector<double> vect;
	for (int j(0);j<i;j++){
		vect[j] = getDistance(units[i], units[j]);
	}
	return vect;
}

	
const double UPGMA::getDistance(Unit a, Unit b)
{
	double distance=0;
	if (a.typeID()!=b.typeID()){
		distance=1000000;
	}
	distance+=a.getDistanceSqToUnit(b, b.firstTimeFree())*distMultiplier;//*1000
	distance+= abs (a.currentHP()-b.currentHP())*hpMultiplier;
		
	return distance;
}
		
