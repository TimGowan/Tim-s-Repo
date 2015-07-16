/********************************************************************************
Author: Tim Gowan
Date: 26 June, 2015
Summary: 
	A simplified version of the ID3 Machine Learning Classification Algorithm. Uses Conditional Entropy 
	and Gain to determine next-step partitioning of a set of partition.	

Command-Line Arguments format:  main.cpp -[Dataset Filename] -[Partion Filename] -[Output Filename]

Input File Description:
	The dataset is described in a file (named "dataset-1.txt"" containing only integer numbers. The first 
	number in the file is m,the number of instances; the second is n, the number of features. These two 
	numbers are followed by mn feature values. To simplify the project we will assume that all attributes 
	are Boolean, represented by “0” or “1”. The last attribute is taken as the target attribute. Example:
	--------------------------
	10 4
	0 0 0 0
	0 0 1 0
	1 0 1 0
	1 1 0 1
	0 0 1 1
	0 1 1 1
	1 1 1 1
	1 1 1 0
	0 0 1 0
	1 0 1 0
	-------------------------
	A partition is described in a text file (named "partition-2.txt), containing only integer numbers. For a situation where there
	are m instances, a partition is described by a sequence of numbers in the range 1 . . . m.
	The first number in each partition is the partition id, and each partition is specified in a separate
	line
	With 10 instances, here is the file representing the partition {1,10}, {2,3,4,5}, {6,7,8,9}.
	The first partition has id=1, the second id=7, and the id of the third is 2. Example: 
	--------------------------
	1 1 10
	7 2 3 4 5
	2 6 7 8 9
	-------------------------
*********************************************************************************/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <math.h>

//Prototypes
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);
int numExamples(std::string filename);
int numLabels(std::string filename);
int** processDataset(std::string filename);
std::vector<std::vector<int>> processPartitions(std::string filename);
std::vector<std::vector<int>> splitPartition(int** dataset, std::vector<int> partition, int examplesCount, int labelsCount, int labelIndex);
std::vector<std::vector<int>> getPartitionDataVector(int** dataset, std::vector<int> partition, int examplesCount, int labelsCount);
int target_label_count(int** dataset, int size);
int** getPartitionDataset(int** dataset, std::vector<int> partition, int examplesCount, int labelsCount);
int label_count(int** dataset, int label_index);
int label_count(int** dataset, int label_index, int size);
double divide(double dividend, double divisor);
double calc_entropy(std::vector<std::double_t> p_set);
double calc_conditional_entropy(std::vector<std::double_t> p_set, std::vector<std::double_t> e_set);
int printPartition(std::vector<std::vector<int>> p);
int  printPartition(std::vector<std::vector<int>> p, int labelIndex);

//*****************Main**************************
using namespace std;
int main(int argc, char* argv[])
{
	int**		dataset;
	int			examplesCount;
	int			labelsCount;
	std::string dataset_filename;
	std::string partitions_filename;
	std::string output_filename;

	if (argc!=3){
		dataset_filename = "dataset-1.txt";
		partitions_filename = "partition-1.txt";
	//	partitions_filename = "partition-2.txt";  //root node testing partition
		output_filename = "output.txt";
	}
	else{
		dataset_filename = argv[0];
		partitions_filename = argv[1];
		output_filename = argv[2];
	}

	std::vector<std::vector<int>> partitions;

	//Get Dataset
	dataset = processDataset(dataset_filename);
	examplesCount = numExamples(dataset_filename);
	labelsCount = numLabels(dataset_filename);

	// Printing contents of dataset file for posterity
	std::cout << "examplesCount= " << examplesCount << "\nlabelsCount= " << labelsCount<< endl;
	std::cout << "Dataset Contents: " << endl;
	for (int i = 0; i < examplesCount; i++){
		for (int j = 0; j < labelsCount; j++){
			std::cout << dataset[i][j] << ' ';
		}
		std::cout << endl;
	}

	//Get Partitions
	partitions = processPartitions(partitions_filename);

	//Printing contents of partitions file for posterity
	std::cout <<endl;
	for (int i = 0; i < partitions.size(); i++){
		std::cout << "Partition" << partitions[i][0] << " contains ( ";
		for (int j = 1; j < partitions[i].size(); j++){
			std::cout << partitions[i][j] << ' ';
		}
		std::cout <<")" << endl;
	}
	std::cout << endl;


	//*****************Calculating Target Entropy*************************

	std::vector<std::double_t>		p_set;
	std::vector<std::double_t>		e_set;
	std::vector<std::double_t>		e2_set;
	std::vector<std::double_t>		p2_set;
	int**							partitionDataset;
	int								labelIndex = 1;
	int								indexedLabelCount;
	int								targetLabelCount;
	int								temp;
	std::vector<std::vector<int>>	candidateSubpartition; 
	double							candidate_entropy;
	double							candidate_conditional_entropy;
	int								currentIndex;

	//Looping through each partition to calculate Gain(maximize) or Conditional_Entropy(minimize)
	for (int i = 0; i < partitions.size(); i++){

		partitionDataset = getPartitionDataset(dataset, partitions[i], examplesCount, labelsCount); // getting dataset for instances found in each partition
		


		int const targetLabelCount = target_label_count(partitionDataset, partitions[i].size()-1);

		if (targetLabelCount == 0 || targetLabelCount == partitions[i].size()){
			std::cout << "All instances in partition"<< partitions[i][0] <<" have same value for target attribute." << endl;
		}
		else{

			p2_set.push_back(divide(targetLabelCount, partitions[i].size()));
			p2_set.push_back(divide(partitions[i].size() - targetLabelCount, partitions[i].size())); //Saving ratios of Target Label to Total

			//*****Compute Gain/Entropy*******

			//Looping through each attribute label
			for (int j = 0; j < sizeof(partitionDataset[0]); j++){
				candidateSubpartition = splitPartition(dataset, partitions[i], examplesCount, labelsCount, j); 
				cout <<endl<< "Splitting with candidate label #" << j <<"..."<< endl;
				printPartition(candidateSubpartition);

				temp = partitions[i].size() - 1;


				indexedLabelCount = label_count(partitionDataset, j, partitions[i].size() - 1);
				//std::cout << "Attribute"<< j <<" Count: " << indexedLabelCount <<'/'<<partitions[i].size()-1<< endl;
				if (indexedLabelCount == temp){ // Candidate Entropy will be 1.
					p_set.push_back(0); 
					p_set.push_back(1);
				}
				else{
					p_set.push_back(divide(indexedLabelCount, temp));
					p_set.push_back(divide(temp - indexedLabelCount, temp));
				}
				candidate_entropy = calc_entropy(p_set);
			//	std::cout << "Entropy of Indexed Label: " << candidate_entropy << '\n';
				e_set.push_back(candidate_entropy);   
				p_set.clear();


				candidate_conditional_entropy = calc_conditional_entropy(p2_set, e_set);

			}
			e_set.clear();

		}
		p2_set.clear();

		std::cout << endl;
	}
	std::cout << endl;
	std::cout << endl;
	std::cout << endl;
	std::cout << endl;
	std::cout << endl;
	e_set.clear();
	p_set.clear();
	p2_set.clear();


	p2_set.push_back(.75);
	p2_set.push_back(.25);

	p_set.push_back(.33333);
	p_set.push_back(.66666);

	e_set.push_back( calc_entropy(p_set));
	e_set.push_back(1);

	candidate_entropy = calc_conditional_entropy(p2_set, e_set);
	std::cout << "Lowest Conditional Entropy: " << candidate_entropy << '\n';



	return 0;
}

double calc_conditional_entropy(std::vector<std::double_t> p_set, std::vector<std::double_t> e_set){
	//p_set = set of probabilities, e_set = set of entropies
	double conditionalEntropy = 0;
	if (p_set.size() == e_set.size())
		for (int i = 0; i < p_set.size(); i++){
			conditionalEntropy += p_set[i] * e_set[i];
		}
	else
				cout << "                          p_set size =" << p_set.size() << ", e_set size = " << e_set.size()<< endl;

			std::cout << "Conditional Entropy of Indexed Label: " << conditionalEntropy << '\n';
		return conditionalEntropy;
}
int  printPartition(std::vector<std::vector<int>> p, int labelIndex){
	//Printing contents of partitions file for posterity
	std::cout << endl;
	for (int i = 0; i < p.size(); i++){
		std::cout << "Candidate Partition" << p[i][0] << " contains ( ";
		for (int j = 1; j < p[i].size(); j++){
			std::cout << p[i][j] << ' ';
		}
		std::cout << ") and was split using label index of "<< labelIndex<< endl;
	}
	std::cout << endl;
	return 0;
}
int  printPartition(std::vector<std::vector<int>> p){
	//Printing contents of partitions file for posterity
	std::cout << endl;
	for (int i = 0; i < p.size(); i++){
		std::cout << "Candidate Partition" << p[i][0] << " contains ( ";
		for (int j = 1; j < p[i].size(); j++){
			std::cout << p[i][j] << ' ';
		}
		std::cout << ")" << endl;
	}
	std::cout << endl;
	return 0;
}

double calc_entropy(std::vector<std::double_t> e_set){
	//where e_set is the set of probabilities to be used as input in summation formula for entropy value
	double entropy = 0;
	for (int i = 0; i < e_set.size(); i++)
		if (e_set[i] == 0)
			entropy += 0; //Avoid divide by 0
		else
			entropy += e_set[i] * log2(1 / e_set[i]); // Calculate entropy and sum
	return entropy;
}

std::vector<std::vector<int>> splitPartition(int** dataset, std::vector<int> partition, int examplesCount, int labelsCount, int labelIndex){
	std::vector<std::vector<int>> subPartitions;
	std::vector<std::vector<int>> partitionDataVector;
	std::vector<int> temp0;
	std::vector<int> temp1;
	int** partitionDataset;

	temp0.push_back(-rand() % 999); // Generate unique (hopefully) ID
	temp1.push_back(-rand() % 999);
	while (temp1[0] == temp0[0]){// ensure ID is unique per subpartition
		temp1.clear();
		temp1.push_back(-rand() % 999);
	}


	
	partitionDataset = getPartitionDataset(dataset, partition, examplesCount, labelsCount);
	for (int i = 0; i < examplesCount; i++)
		for (int j = 1; j < partition.size(); j++){ // loop through partition
			if (partition[j] == i) //if index of dataset is inside parition
				if (dataset[i][labelIndex] == 0) // if we found a 0
					temp0.push_back(partition[j]); //save index of dataset found in partition
				else
					temp1.push_back(partition[j]);
		}
	

	subPartitions.push_back(temp0);
	subPartitions.push_back(temp1);
	temp0.clear();
	temp1.clear();

	return subPartitions;
}

std::vector<std::vector<int>> evaluateCandidateAttributes(int** dataset, std::vector<int> partition, int examplesCount, int labelsCount){
	std::vector<std::vector<int>>	subPartitions;
	double							conditional_entropy = 0;
	int**							partition_dataset = new int*[partition.size()];
	int								targetCount = 0;

	for (int i = 0; i <= partition.size() - 1; i++){ 

	}


	//Get data of examples in partition ONLY 
	for (int i = 0; i <= examplesCount; i++){
		for (int j = 0; j < partition.size(); j++)
			if (i == partition[j]){
				partition_dataset[j] = dataset[i - 1];
			}
	}



	return subPartitions;
}
// *********************************************************************************BUGGED.
int** getPartitionDataset(int** dataset, std::vector<int> partition, int examplesCount, int labelsCount){ 
	double			conditional_entropy = 0;
	int**			partition_dataset = new int*[partition.size()];
	int				targetCount = 0;


	//Get data of examples in partition ONLY
	for (int i = 0; i <= examplesCount; i++){
		for (int j = 0; j < partition.size(); j++)
			if (i == partition[j]){
				partition_dataset[j] = dataset[i-1];
			}
	}
	/*
	//Print contents of partition dataset
	std::cout << "Partition"<<partition[0] <<" contains Dataset Contents: " << endl;
	for (int i = 1; i < partition.size(); i++){
		for (int j = 0; j < labelsCount; j++){
			std::cout << partition_dataset[i][j] << ' ';
		}
		std::cout << endl;
	}
	*/

	return partition_dataset;
}

int label_count(int** dataset, int label_index, int size){ 
	// counting number of hits of Target Label
	int labelCount = 0;
	for (int i = 1; i <=size; i++){
		labelCount += dataset[i][label_index]; 
	}
	//std::cout << "labelCount: " << labelCount << endl;
	return labelCount;
}

int target_label_count(int** dataset, int size){
	// counting number of hits of Target Label
	int labelCount = 0;
	for (int i = 1; i <= size; i++){
		labelCount += dataset[i][sizeof(dataset[0])-1];
	}
	//std::cout << "labelCount: " << labelCount << endl;
	return labelCount;
}

std::vector<std::vector<int>> processPartitions(std::string filename){
	//*****************Processing Partition*************************
	vector<std::string>				tokenized_string;
	ifstream						partition_old;
	std::string						line;
	std::vector<std::vector<int>>	partitions;
	std::vector<int>				partition;

	
	partition_old.open(filename);
	if (partition_old.is_open())
	{
		int num_lines = 0;
		int temp;
		while (getline(partition_old, line))
		{
			num_lines++;
			tokenized_string = split(line, ' ');
			for (int i = 0; i < tokenized_string.size(); i++){
				istringstream(tokenized_string[i]) >> temp;
				partition.push_back(temp);
			}
			partitions.push_back(partition);
			partition.clear();
		}

	}
	else
		cout << "Partition file cant be opened." << '\n';

	partition_old.close();

	return partitions;
}

int numExamples(std::string filename){
	ifstream					datasetStream;
	int							num;
	std::string					line;
	vector<std::string>			tokenized_string;

	datasetStream.open(filename);

	getline(datasetStream, line);
	tokenized_string = split(line, ' ');

	istringstream(tokenized_string[0]) >> num;
	datasetStream.close();
	return num;
}

int numLabels(std::string filename){
	ifstream					datasetStream;
	int							num;
	std::string					line;
	vector<std::string>			tokenized_string;

	datasetStream.open(filename);

	getline(datasetStream, line);
	tokenized_string = split(line, ' ');


	istringstream(tokenized_string[1]) >> num;

	datasetStream.close();
	return num;
}

int** processDataset(std::string filename){
	//*****************Processing Dataset**************************
	vector<std::string>			tokenized_string;
	int							numExamples; // Number of examples in dataset
	int							numLabels; //Number of labels for classification (with last label being the target label) for each example in dataset.
	std::string					line;
	ifstream					datasetStream;

	datasetStream.open(filename);


	getline(datasetStream, line);
	tokenized_string = split(line, ' ');

	istringstream(tokenized_string[0]) >> numExamples;
	istringstream(tokenized_string[1]) >> numLabels;



	//Dynamically allocate 2d Array dataset[Example][Label] and place values into array
	int** dataset = new int*[numExamples];
	for (int i = 0; i < numExamples; ++i){
		dataset[i] = new int[numLabels];
		getline(datasetStream, line);
		tokenized_string = split(line, ' ');
		for (int j = 0; j < numLabels; j++){
			istringstream(tokenized_string[j]) >> dataset[i][j];
			//std::cout << dataset[i][j] << ' ';
		}
	}

	datasetStream.close();

		return dataset;

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

double divide(double dividend, double divisor){
	double quotient;
	quotient = dividend / divisor;
	return quotient;
}
