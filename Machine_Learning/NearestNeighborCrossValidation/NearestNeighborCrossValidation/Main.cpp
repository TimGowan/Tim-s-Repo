/*
// Author: Tim Gowan
// Date: 7/12/2015
// Description: A program to implement cross validation evaluation of the k-nearest-neighbors algorithm.
*/


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <math.h>
#include <algorithm>
#include < queue>

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
data nearestNeighborAlgorithm(unsigned int k, data input);
char nearestNeighborsLabel(std::vector<example> nearestNeighbors, int k);
std::vector<int> determineFolds(int numFolds, int numExamples);
std::vector<example*> crossValidate(data input, CV_Permutations CVFile, int permutationIndex);
char** printAsGrid(int numCols, int numRows, std::vector < example> e);

using namespace std;
int main(int argc, char* argv[])
{

	data						baseData;
	CV_Permutations				CVFile;
	std::vector<data>			kNearestNeighbors;


	std::string					CV_filename; //CV = Cross Validation
	std::string					training_data_filename;
	std::string					output_filename;

	if (argc != 3){
		CV_filename = "CV-1.txt";
		training_data_filename = "dataset-1.txt";
		output_filename = "output.txt";
	}
	else{
		CV_filename = argv[0];
		training_data_filename = argv[1];
		output_filename = argv[2];
	}

	//reading files for input
	baseData = readDataset(training_data_filename);
	CVFile = readCVFile(CV_filename);

	
	crossValidate(baseData, CVFile, 2);

	//for (int k = 1; k <= 5; k++)
		//nearestNeighborAlgorithm(k, baseData);
	//folds = determineFolds(CVFile.numFolds, CVFile.numExamples);



}
std::vector<example*> crossValidate(data input, CV_Permutations CVFile, int permutationIndex){
	std::vector<int>			numFolds;
	std::queue <example>		shuffledExamples;
	int*						permutation; // the "random" set of integers that make up the order of the shuffle
	std::vector<example*>		folds;

	std::cout << endl;
	numFolds = determineFolds(CVFile.numFolds, CVFile.numExamples);
	//numFolds = determineFolds(CVFile.numExamples, CVFile.numExamples);  //Leave-one-out approach where numFolds == numExamples

	//determine order of 'shuffle'
	permutation = CVFile.permutations[permutationIndex];
	std::cout << "Selected Shuffle: " << endl;
	for (int i = 0; i < CVFile.numExamples; i++){ // Loop through each example in permutation
		//std::cout << permutation[i] << ' ';
		for (int j = 0; j < input.examples.size(); j++){
			if (input.examples[j].id == permutation[i]){
				cout << input.examples[j].id << ' ';
				shuffledExamples.push(input.examples[j]); //placing examples in 'shuffled' order
			}
		}
	}

	std::cout << endl;
	std::cout << "Fold Subdivisions: " << endl;


	for (int i = 0; i < numFolds.size(); i++){
		example* temp = new example[numFolds[i]];
		for (int j = 0; j < numFolds[i]; j++){
			temp[j] = shuffledExamples.front(); // take object
			shuffledExamples.pop(); 
			std::cout << temp[j].id << ' ';
		}
		folds.push_back(temp);
		std::cout << endl;
	}
	
	return folds;

}

std::vector<int> determineFolds(int numFolds, int numExamples){
	std::vector < int > folds;
	int temp;
	int remainder;

	//int* folds = new int[numFolds]; // allocate an array with one slot for each fold

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
					folds.push_back(temp + 1);
					remainder--; //Distribute remainder;
				}
				else
					folds.push_back(temp);
			}
	}
	for (int i = 0; i < folds.size(); i++)
		cout << "fold" << i << " has " << folds[i] << endl;;

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

char nearestNeighborAlgorithm(unsigned int k, example unlabeled, std::vector<example> examples){
	std::vector<example>		nearestNeighbors; //List to keep track of nearest neighbors
	example						temp;
	char						tLabel;

	for (int j = 0; j < examples.size(); j++){//loop through all examples and calculate nearest neighbors
		temp = examples[j];
		temp.distance = euclidianDistance(examples[j], unlabeled);
		nearestNeighbors.push_back(temp);
	}
	tLabel = nearestNeighborsLabel(nearestNeighbors, k);
	nearestNeighbors.clear();

	return tLabel;
}

data nearestNeighborAlgorithm(unsigned int k, data input){
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

	std::cout << "k = " << k << endl;

	output.datasetGrid = printAsGrid(output.numCols, output.numRows, output.examples);
	return output;
}

char nearestNeighborsLabel(std::vector<example> nearestNeighbors, int k){
	int			countPos = 0;
	int			countNeg = 0;
	char		assignedLabel;

	std::sort(nearestNeighbors.begin(), nearestNeighbors.end(), by_distance); // sort by distance


	for (int k = 0; k < nearestNeighbors.size(); k++){ //printing contents of nearestNeighbor
//	std::cout << nearestNeighbors[k].x1 << " | " << nearestNeighbors[k].x2 << " | " << nearestNeighbors[k].y << " | " << nearestNeighbors[k].distance << endl;
	}
	
	for (int i = 0; i < k; i++){
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

		cout << CV.numFolds << " folds, "<< CV.numExamples <<" examples, "<< CV.numShuffles << " shuffles"<<endl;

		int**	permutations = new int*[CV.numShuffles];


		for (int i = 0; i < CV.numShuffles; i++){
			permutations[i] = new int[CV.numExamples];
			getline(fstream, line);
			tokenized_string = split(line, ' ');
			for (int j = 0; j < CV.numExamples; j++){
				istringstream(tokenized_string[j]) >> permutations[i][j];
				cout << permutations[i][j] << ' ';
			}
			cout << endl;
			
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
				std::cout << dataset[i][j] << ' ';
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
			std::cout << endl;

		}
		datasetStream.close();
		data.examples = exampleVector;
		data.unlabeleds = unlabledVector;
		data.datasetGrid = dataset;

		exampleVector.clear();
		unlabledVector.clear();
		return data;
	}
	else{
		cout << "dataset not open" << endl;
	}
}

bool by_distance(const example& lhs, const example& rhs) { return lhs.distance < rhs.distance; }

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