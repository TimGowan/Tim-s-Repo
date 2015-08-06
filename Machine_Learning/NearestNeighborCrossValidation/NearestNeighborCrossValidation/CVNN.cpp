/*
// Author: Tim Gowan
// Date: 7/12/2015
// Description: A program to implement cross validation evaluation of the k-nearest-neighbors algorithm based upon given input files.
// Using the cross validation, standard deviation(sigma) is then computed off of the number of incorrectly labeled inputs.
*/


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <queue>

//Struct definitions
struct example{
	int			x1; // index of column
	int			x2;	// index of row
	char		y;	//given label ('.', '+'or '-')
	double		distance = 0; // Used for Nearest Neighbor Algorithm
	int			id;	// Used for easy cross-validation folding
};

struct data{
	std::vector<example>		examples; //list of examples with a label
	std::vector<example>		unlabeleds; // list of examples without a label
	char**						datasetGrid;
	int							numRows;
	int							numCols;
};

struct CV_Permutations{
	int			numShuffles;
	int			numFolds;
	int			numExamples;
	int**		permutations; // Pregenerated "random" permutations

};

//Prototypes
bool by_distance(const example& lhs, const example& rhs);	// Ordering function to compare distances between examples to be used by std::sort in nearest neighbor algorithm
bool by_x2_x1(const example& lhs, const example& rhs);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);
data readDataset(std::string filename);
CV_Permutations readCVFile(std::string filename);
int euclidianDistance(example e, example u);
char nearestNeighborAlgorithm(unsigned int k, example unlabeled, std::vector<example> examples);
char nearestNeighborsLabel(std::vector<example> nearestNeighbors, int x);
std::vector<int> determineFolds(int numFolds, int numExamples);
std::vector<std::vector<example> > foldData(data input, CV_Permutations CVFile, int permutationIndex);
char** printAsGrid(int numCols, int numRows, std::vector < example> e);
double CrossValidate(std::vector<std::vector<example>> in, CV_Permutations CVFile, int x);
double eCalc(data baseData, CV_Permutations CVFile, int k);
double VCalc(data baseData, CV_Permutations CVFile, int k);
data gridMe(unsigned int k, data input);

using namespace std;
int main(int argc, char* argv[])
{
	data						baseData;
	CV_Permutations				CVFile;

	unsigned int				k = 5; // Number of Nearest Neighbors to account for.
	double						V=0;

	std::string					CV_filename; //CV = Cross Validation
	std::string					training_data_filename;

	if (argc != 2){
		CV_filename = "CV-1.txt";
		training_data_filename = "dataset-1.txt";
	}
	else{
		CV_filename = argv[0];
		training_data_filename = argv[1];
	}
	//reading files for input
	baseData = readDataset(training_data_filename);
	CVFile = readCVFile(CV_filename);

	//Calculate Values
	for (k = 1; k <= 5; k++){ //Calculate for k-nearest-neighbors = {1,2,3,4,5}
		cout << "k=" << k;
		cout << "   e=" << eCalc(baseData, CVFile, k);
		V = VCalc(baseData, CVFile, k); // V = Variance
		cout << "   sigma=" << sqrt(V); // Sigma = Standard Deviation.
		cout << endl;
		gridMe(k, baseData);
	}
}
double VCalc(data baseData, CV_Permutations CVFile, int k){
	const int					t = CVFile.numShuffles;
	double						V = 0;
	double						e = eCalc(baseData, CVFile, k);

	for (int i = 0; i < t ; i++){ //compute Variance
		double temp = CrossValidate(foldData(baseData, CVFile, i), CVFile, k)-e;
		V += (pow(temp, 2)) / (t - 1);
	}
	return V;
}
double	eCalc(data baseData, CV_Permutations CVFile, int k){
	double						e = 0;
	const int					t = CVFile.numShuffles;

	for (int i = 0; i < t; i++){ // Cross validate across each shuffle to calculate e.
		e += CrossValidate(foldData(baseData, CVFile, i), CVFile, k);
	}
	e = e / t;
	return e;
}

double CrossValidate(std::vector<std::vector<example>> in, CV_Permutations CVFile, int k){
	std::vector<example>	training;
	std::vector<example>	testing;

	int						numErrors; // Count of errors for each fold
	int						totalErrors = 0;
	double					classifierError; // totalErrors/CVFile.numExamples

	//cout <<endl<< "In cross validation..." << endl;
	for (int i = 0; i < in.size(); i++){ // loop through each fold to cross validate all folds
		numErrors = 0;
		for (int j = 0; j < in.size(); j++){ // loop through each fold to cover all folds
			if (i == j){
				//cout << "Test me:" << endl;
				for (int z = 0; z < in[j].size(); z++){ // loop through each example in the fold to calculate e.
				//	cout <<in[j][z].id<< ' ' ;
					testing.push_back(in[j][z]);
				}
			}
			else{
				//cout << "Train me:" << endl;
				for (int z = 0; z < in[j].size(); z++){ // loop through each example in the fold to calculate e.
				//	cout << in[j][z].id << ' ';
					training.push_back(in[j][z]);
				}
			}
		//	cout<< endl;
			// Run Nearest Neighbor on each example in 'testing' based on examples in 'training'

		}
		//cout << "training.size() =" << training.size() << endl;
		//cout << "testing.size() =" << testing.size() << endl;
		for (int z = 0; z < testing.size(); z++){
			if (testing[z].y != nearestNeighborAlgorithm(k, testing[z], training))
				numErrors++;
		}
		//std::cout <<endl <<"numErrors in this fold:" << numErrors << endl;

		totalErrors += numErrors;
		numErrors = 0;
		testing.clear();
		training.clear();	
	}
	//std::cout << endl << "totalErrors in Cross Validation:" << totalErrors << endl;
	classifierError = totalErrors / (double)CVFile.numExamples;
	//std::cout << endl << "classifierError in Cross Validation:" << classifierError << endl;

	return classifierError;
}

std::vector<std::vector<example> > foldData(data input, CV_Permutations CVFile, int permutationIndex){
	std::vector<int>						numFolds;
	std::queue <example>					shuffledExamples;
	int*									permutation; // the "random" set of integers that make up the order of the shuffle
	std::vector<std::vector<example>>		folds;
	std::vector<example>					temp;


	numFolds = determineFolds(CVFile.numFolds, CVFile.numExamples);
	//numFolds = determineFolds(CVFile.numExamples, CVFile.numExamples);  //Leave-one-out test case where numFolds == numExamples



	//determine order of 'shuffle'
	permutation = CVFile.permutations[permutationIndex];
	//std::cout<<endl << "Selected Shuffle: " << endl;
	for (int i = 0; i < CVFile.numExamples; i++){ // Loop through each example in permutation
		//std::cout << permutation[i] << ' ';
		for (int j = 0; j < input.examples.size(); j++){
			if (input.examples[j].id == permutation[i]){
			//	cout << input.examples[j].id << ' ';
				shuffledExamples.push(input.examples[j]); //placing examples in 'shuffled' order
			}
		}
	}

	//std::cout<<endl << "Cross Validation Subdivisions: " << endl;
	//placing correct examples in fold.
	for (int i = 0; i < numFolds.size(); i++){
		for (int j = 0; j < numFolds[i]; j++){
			temp.push_back(shuffledExamples.front()); // take object
			shuffledExamples.pop(); 
			//std::cout << temp[j].id << ' ';
		}
		folds.push_back(temp);
		temp.clear();
	//	std::cout << endl;
	}
	
	return folds;

}

std::vector<int> determineFolds(int numFolds, int numExamples){
	std::vector < int > folds;
	int temp;
	int remainder;


	if (numExamples%numFolds == 0){ // if the examples divide evenly
		temp = numExamples / numFolds;
		for (int i = 0; i < numFolds; i++){
			folds.push_back(temp);
		}
	}
	else{
		remainder = numExamples%numFolds;
		//cout << "remainder: " << remainder<< endl;
		temp = (int)numExamples / numFolds;
			for (int i = 0; i < numFolds; i++){
				if (remainder > 0){
					folds.push_back(temp+1);
					remainder--; //Distribute remainder;
				}
				else
					folds.push_back(temp);
			}
	}
	std::reverse(folds.begin(), folds.end()); // Invert order to satisfy project constraint

	//for (int i = 0; i < folds.size(); i++)
		//cout << "fold" << i << " has " << folds[i] << endl;

	return folds;
}

char** printAsGrid(int numCols, int numRows, std::vector < example> e){

	//transforming table into grid.
	std::sort(e.begin(), e.end(), by_x2_x1); //sort for easy grid insertion
	for (int i = 0; i < e.size(); i++){
		e[i].id = i;
		//	std::cout << output.examples[i].x1 << " | " << output.examples[i].x2 << " | " << output.examples[i].y << endl;
	}
	//allocating memory
	char** outputGrid = new char*[numRows];
	for (int i = 0; i < numRows; i++)
		outputGrid[i] = new char[numCols];
	//std::cout<< output.numRows << output.numCols<< endl;
	
	for (int j = 0; j < numRows; j++){
		for (int k = 0; k < numCols; k++){
			for (int i = 0; i < e.size(); i++)
				if (e[i].x2 == j && e[i].x1 == k)
					outputGrid[j][k] = e[i].y;
			std::cout << outputGrid[j][k] << ' ';
		}
		std::cout << endl;
	}
	return outputGrid;
}

char nearestNeighborAlgorithm(unsigned int x, example unlabeled, std::vector<example> examples){
	std::vector<example>		nearestNeighbors; //List to keep track of nearest neighbors
	example						temp;
	char						tLabel= '.';
	
	for (int j = 0; j < examples.size(); j++){//loop through all examples and calculate nearest neighbors
		temp = examples[j];
		temp.distance = euclidianDistance(examples[j], unlabeled);
		nearestNeighbors.push_back(temp);
	}
	//cout << "nearestNeighbors.size() =" << nearestNeighbors.size() << endl;
	tLabel = nearestNeighborsLabel(nearestNeighbors, x);
	nearestNeighbors.clear();
	
	return tLabel;
}

data gridMe(unsigned int k, data input){
	std::vector<example>		nearestNeighbors; //List to keep track of nearest neighbors
	example						temp;
	data						output;
	char						tLabel;

	output.numCols = input.numCols;
	output.numRows = input.numRows;
	output.examples = input.examples;

	for (int i = 0; i < input.unlabeleds.size(); i++){ //loop through all unlabeled items
		temp = input.unlabeleds[i];
		temp.y = nearestNeighborAlgorithm(k, input.unlabeleds[i], input.examples);
		output.examples.push_back(temp);
	}

	output.datasetGrid = printAsGrid(output.numCols, output.numRows, output.examples);
	return output;
}

char nearestNeighborsLabel(std::vector<example> nearestNeighbors, int x){
	int			countPos = 0;
	int			countNeg = 0;
	char		assignedLabel;
	int			temp = x; // x is the number of nearest neighbors we are supposed to take into account

	std::sort(nearestNeighbors.begin(), nearestNeighbors.end(), by_distance); // sort by distance

	
	for (int k = 0; k < nearestNeighbors.size(); k++){ //printing contents of nearestNeighbor
	//std::cout << nearestNeighbors[k].x1 << " | " << nearestNeighbors[k].x2 << " | " << nearestNeighbors[k].y << " | " << nearestNeighbors[k].distance << endl;
	}
	while (nearestNeighbors.size() < temp)
		temp--;
	for (int i = 0; i < temp; i++){
			if (nearestNeighbors[i].y == '+')
				countPos++;
			else
				countNeg++;
	}
	if (countNeg >= countPos)
		assignedLabel = '-';
	else
		assignedLabel = '+';
	//std::cout << "+: " << countPos << "   -: " << countNeg << "   assignedLabel: " << assignedLabel << endl;

		return assignedLabel;
}

int euclidianDistance(example e, example u){
	unsigned int t1 = e.x1 - u.x1;
	unsigned int t2 = e.x2 - u.x2;
	int	distance = t1*t1+t2*t2;
	//cout << distance << endl;
	return distance;
}

CV_Permutations readCVFile(std::string filename){
	ifstream					fstream;
	std::string					line;
	vector<std::string>			tokenized_string;
	CV_Permutations				CV;
	

	fstream.open(filename);
	if (fstream.is_open()){
		getline(fstream, line);
		tokenized_string = split(line, ' ');

		istringstream(tokenized_string[0]) >> CV.numFolds;
		istringstream(tokenized_string[1]) >> CV.numExamples;
		istringstream(tokenized_string[2]) >> CV.numShuffles;

	//	cout << CV.numFolds << " folds, "<< CV.numExamples <<" examples, "<< CV.numShuffles << " shuffles"<<endl;

		int**	permutations = new int*[CV.numShuffles];


		for (int i = 0; i < CV.numShuffles; i++){
			permutations[i] = new int[CV.numExamples];
			getline(fstream, line);
			tokenized_string = split(line, ' ');
			for (int j = 0; j < CV.numExamples; j++){
				istringstream(tokenized_string[j]) >> permutations[i][j];
	//			cout << permutations[i][j] << ' ';
			}
	//		cout << endl;
			
		}
		CV.permutations = permutations;

	}

	return CV;
}

data readDataset(std::string filename){
	//*****************Processing Dataset**************************
	vector<std::string>			tokenized_string;
	int							numRows; // Number of rows in dataset
	int							numCols; //Number of columns in dataset
	int							eCount = 0;
	int							uCount = 0;
	std::string					line;
	ifstream					datasetStream;
	std::vector<example>		exampleVector;
	std::vector<example>		unlabledVector;
	example						eTemp;
	example						uTemp;
	data						data;

	datasetStream.open(filename);

	if (datasetStream.is_open())
	{ 
		getline(datasetStream, line);
		tokenized_string = split(line, ' ');


		istringstream(tokenized_string[0]) >> numRows;
		istringstream(tokenized_string[1]) >> numCols;
		data.numRows = numRows;
		data.numCols = numCols;
		//std::cout << numRows << ' ' << numCols << endl;

		//Dynamically allocate 2d Array dataset[Rows][Cols] and place values into array
		char** dataset = new char*[numRows];
		
		for (int i = 0; i < numRows; ++i){
			dataset[i] = new char[numCols];
			getline(datasetStream, line);
			tokenized_string = split(line, ' ');
			for (int j = 0; j < numCols; j++){
				istringstream(tokenized_string[j]) >> dataset[i][j];
				if (dataset[i][j] != '+' && dataset[i][j] != '.' )
					dataset[i][j] = '-';
				//std::cout << dataset[i][j] << ' ';
				if (dataset[i][j] != '.'){ // If example is labeled, push into exampleVector
					eTemp.x1 = j;
					eTemp.x2 = i;
					eTemp.y = dataset[i][j];
					eTemp.id = eCount;
					//std::cout << "Example"<<eCount<<":  " << eTemp.x1 << "|" << eTemp.x2 << "|" << eTemp.y << endl;
					exampleVector.push_back(eTemp);
					eCount++;
				}
				else{//Keep track of unlabled indexes to calculate distances
					uTemp.x1 = j;
					uTemp.x2 = i;
					//std::cout << "Unlabled"<<uCount<<": " << uTemp.x1 << "|" << uTemp.x2 << endl;
					unlabledVector.push_back(uTemp); 
					uCount++;
				}

			}
			//std::cout << endl;

		}
		datasetStream.close();
		data.examples = exampleVector;
		data.unlabeleds = unlabledVector;
		data.datasetGrid = dataset;

		exampleVector.clear();
		unlabledVector.clear();

		delete[] dataset;
		return data;
	}
	else{
		cout << "dataset not open" << endl;
	}
}

bool by_distance(const example& lhs, const example& rhs) { 
	if (lhs.distance == rhs.distance) // If distance is tied, favor '-' as described in requirements
		return lhs.y > rhs.y;
	else
		return lhs.distance < rhs.distance;
}

bool by_x2_x1(const example& lhs, const example& rhs) { //Sorting by 2 columns
	if (lhs.x2 == rhs.x2)
		return  lhs.x1 < rhs.x1;
	else
		return lhs.x2<rhs.x2;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}//function to split string and return in premade vector

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}//function to split string and return in new vector