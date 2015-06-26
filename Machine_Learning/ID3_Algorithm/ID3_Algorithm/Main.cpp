#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <math.h>



//Prototypes
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

double divide(double dividend, double divisor);
double calc_entropy(std::vector<std::double_t> p_set);

using namespace std;
int main(int argc, char* argv[])
{

	//Variable Declarations
	std::string line;

	ifstream datasetStream;
	ifstream partition_old;
	ofstream partition_new;
	vector<std::string> tokenized_string;
	

	int numExamples; // Number of examples in dataset
	int numLabels; //Number of labels for classification (with last label being the target label) for each example in dataset.


	//*****************Processing Dataset**************************
	cout << "Opening Dataset File..."<< '\n';
	datasetStream.open("dataset-1.txt");
	if (datasetStream.is_open())
	{
		getline(datasetStream, line);
			tokenized_string = split(line, ' ');

			istringstream(tokenized_string[0]) >> numExamples;
			istringstream(tokenized_string[1]) >> numLabels;

			cout << "numExamples: " << numExamples << "\nnumLabels: " << numLabels; // Print for posterity
			
			//Dynamically allocate 2d Array dataset[Example][Label] and place values into array
			int** dataset = new int*[numExamples];
			for (int i = 0; i < numExamples; ++i){
				dataset[i] = new int[numLabels];
				getline(datasetStream, line);
				tokenized_string = split(line, ' ');
				cout << '\n';
				for (int j = 0; j < numLabels; j++){
					istringstream(tokenized_string[j]) >> dataset[i][j];
					cout << dataset[i][j] << ' ';
				}
			}

			datasetStream.close();
	}
	else
		cout << "Dataset file cant be opened." << '\n';
	datasetStream.close();

	//*****************Processing Partition*************************
	cout << '\n' << "Opening partition File..." << '\n';
	partition_old.open("partition-1.txt");
	if (partition_old.is_open())
	{
		int num_lines = 0;
		while (getline(partition_old, line))
		{
			
			++num_lines;
			tokenized_string = split(line, ' ');
			
			cout << line << '\n';
		}
	}
	else
		cout << "Partition file cant be opened." << '\n';




	cout << "Testing Entropy Function: " << '\n';

	std::vector<std::double_t> p_set;
	//std::vector<double>::const_iterator i;


	//p_set.push_back(.375);
	//p_set.push_back(.625);
	p_set.push_back(divide(1, 4));
	p_set.push_back(divide(3, 4));

	/*
	for (i = p_set.begin(); i != p_set.end(); ++i)
		std::cout << (*i) << std::endl;
	*/
	//Same functionality as above for-loop
	std::cout << p_set[0] << '\n';
	std::cout << p_set[1] << '\n';
	
	cout << calc_entropy(p_set) << '\n';


	return 0;
}


//function to split string and return in premade vector
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) { 
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}
//function to split string and return in new vector
std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

double divide(double dividend, double divisor){
	double quotient;
	quotient = dividend / divisor;
	return quotient;
}

double calc_entropy(std::vector<std::double_t> p_set){
	//where p_set is the set of probabilities to be used as input in summation formula for entropy value
	double entropy = 0;
	for (int i = 0; i < p_set.size(); i++)
		entropy += p_set[i] * log2(1 / p_set[i]); // Calculate entropy and sum
	return entropy;
}

double calc_conditional_entropy(std::vector<std::double_t> p_set, std::vector<std::double_t> class_p_set){
	double conditional_entropy = 0;


	return conditional_entropy;
}