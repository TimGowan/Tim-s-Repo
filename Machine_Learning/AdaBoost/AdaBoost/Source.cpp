
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <queue>

struct example {
	std::vector<double>			x;//a list of n real numbers. These are assumed to be in increasing order
	std::vector < int >			y;//a list of n numbers, each one is either 1 or -1
	std::vector<double>			p;//a list of n nonnegative numbers that sum up to 1
};
struct data{
	int							T;
	int							n;
	example						e;
};
struct hypothesis{
	int							thresholdIndex;
	int							numErrors;
	bool						sign; // 1 = '<', 0 = '>'
	std::vector<int>			errorIndices; // indexes of found errors
};

//Prototypes
data readDataset(std::string filename);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);
std::vector<int> findThresholds(std::vector<int> y);
hypothesis evaluateThresholds(data d, std::vector<int> thresholds);
hypothesis lessThanHypothesis(data d, int threshold);
hypothesis greaterThanHypothesis(data d, int threshold);
hypothesis evaluateHypothesis(data d, int threshold);
double	calculateWeightedTrainingError(data d, hypothesis h);
void printHypothesisClassifier(data d, hypothesis h);
double calculateWeightGoodness(data d, hypothesis h);
double calculateNormalizationFactor(data d, hypothesis h);
std::vector<double> updateProbabilities(data d, hypothesis h);
void determineBoostedClassifier(std::vector<data> d, std::vector<hypothesis> h);
double calculateBoostedClassifierError(std::vector<data> d, std::vector<hypothesis> h);
double calculateBound(std::vector<data> d, std::vector<hypothesis> h);

using namespace std;
void main(int argc, char*argv[]){
	std::string				inputFile;
	data					d;
	data					temp;
	std::vector<data>		iterations;
	std::vector<hypothesis> hypotheses;

	if (argc != 2){
		inputFile = "InputFile.txt";
	}
	else{
		inputFile = argv[0];
	}

	d = readDataset(inputFile);
	temp = d;

	for (int i = 0; i < d.T; i++){

	}
	for (int i = 0; i < d.T; i++){
		hypotheses.push_back(evaluateThresholds(d, findThresholds(d.e.y)));
		iterations.push_back(temp);
		if (calculateWeightedTrainingError(iterations[i], hypotheses[i])<.5){
			std::cout << "Iteration #" << i+1 << ":" << endl;
			printHypothesisClassifier(iterations[i], hypotheses[i]);
			temp.e.p = (updateProbabilities(temp, hypotheses[i]));
			determineBoostedClassifier(iterations, hypotheses);
			calculateBoostedClassifierError(iterations, hypotheses);
			std::cout << "	The boosted classifier error: " << calculateBoostedClassifierError(iterations, hypotheses) << " mistakes"<< endl;
			std::cout << "	The bound on Et: " << calculateBound(iterations, hypotheses) << endl;
		}
		else{
			std::cout << "Iteration #" << i+1 << " cannot find anymore hypotheses" << endl;
		}
	}

}
double calculateBound(std::vector<data> d, std::vector<hypothesis> h){
	double bound = 1;
	for (int i = 0; i < d.size(); i++)
		bound = bound*calculateNormalizationFactor(d[i], h[i]);
	return bound;
}
double calculateBoostedClassifierError(std::vector<data> d, std::vector<hypothesis> h){
	double Et = 0;
	for (int i = 0; i < d.size(); i++){
		h[i].numErrors;
	}
	return Et/d[0].n;
}

void determineBoostedClassifier(std::vector<data> d, std::vector<hypothesis> h){
	string sign;
	std::cout << "	Boosted classifier: ";
	for (int i = 0; i < d.size(); i++){
		if (h[i].sign == 0)
			sign = '>';
		else
			sign = "<=";
		if (calculateWeightGoodness(d[i], h[i]) != 0){
			std::cout << calculateWeightGoodness(d[i], h[i]) << " I( x" << sign << "" << d[i].e.x[h[i].thresholdIndex] << " ) ";
			if (i != d.size() - 1)
				std::cout << " + ";
		}
	}
	std::cout << endl;
}


std::vector<double> updateProbabilities(data d, hypothesis h){
	std::vector<double>		newPs;
	double					temp;
	double					at = calculateWeightGoodness(d, h);
	double					zt = calculateNormalizationFactor(d,h);
	bool					correct;

	for (int i = 0; i < d.n; i++){
		for (int j = 0; j < h.errorIndices.size(); j++)
			if (i == h.errorIndices[j])
				temp = (d.e.p[i] * exp(at)) / zt;
			else
				temp = (d.e.p[i] * exp(-at)) / zt;
		newPs.push_back(temp);
		//std::cout << temp << ' ';
	}
	//std::cout << endl;
	//std::cout<< "Size of newPs: " << newPs.size()<<endl;
	return newPs;
}
double calculateNormalizationFactor(data d, hypothesis h){
	double et = calculateWeightedTrainingError(d, h);
	double zt = 2*sqrt(et*(1-et));
	//std::cout << "zt = " << zt << endl;
	return zt;
}

double calculateWeightGoodness(data d, hypothesis h){
	double et = calculateWeightedTrainingError(d, h);
	double at = .5*log((1 - et) / et);

	//std::cout << "et = " << et << endl;
	//std::cout << "at = " << at << endl;
	return at;
}

double	calculateWeightedTrainingError(data d, hypothesis h){
	double et = 0;
	for (int i = 0; i < h.numErrors; i++){
		et += d.e.p[h.errorIndices[i]];
	}
	//std::cout << "et of " << h.thresholdIndex << " = " << et <<endl;
	return et;
}
void printHypothesisClassifier(data d, hypothesis h){
	string sign;
	if (h.sign == 0)
		sign = '>';
	else
		sign = "<=";
	std::cout << "	The selected weak classifier: x" << sign << d.e.x[h.thresholdIndex] << endl;
	std::cout << "	The weighted training error : " << calculateWeightedTrainingError(d,h) << endl;
	std::cout << "	The weight goodness : " << calculateWeightGoodness(d, h) << endl;
	std::cout << "	The probabilities normalization factor : " << calculateNormalizationFactor(d, h) << endl;
}

hypothesis evaluateThresholds(data d, std::vector<int> thresholds){ //finding best hypothesis (lowest error count)
	hypothesis	current;
	hypothesis	temp = evaluateHypothesis(d, thresholds[0]); //take first hypothesis

	for (int i = 1; i < thresholds.size(); i++){//Loop through thresholds to find ideal hypothesis
		if (temp.numErrors>evaluateHypothesis(d, thresholds[i]).numErrors && calculateWeightedTrainingError(d, evaluateHypothesis(d, thresholds[i]))<.5)
			temp = evaluateHypothesis(d, thresholds[i]);
		else
			temp = temp;
	}
	current = temp;
	return current;
}

hypothesis evaluateHypothesis(data d, int threshold){ //find better hypothesis between '>' and '<'
	hypothesis lessThan = lessThanHypothesis(d, threshold);
	hypothesis greaterThan = greaterThanHypothesis(d, threshold);
	//cout << endl;
	//std::cout << "Evaluating Threshold " << threshold<< endl<<  " <: " << lessThan.numErrors << "    >: " << greaterThan.numErrors << endl;
	if (lessThan.numErrors <= greaterThan.numErrors) //prefer lowest numErrors for each hypothesis
		return lessThan;
	else
		return greaterThan;
}

hypothesis lessThanHypothesis(data d, int threshold){// '<' of threshold
	hypothesis	h;
	h.thresholdIndex = threshold;
	h.sign = 1;// 1 = '<', 0 = '>'
	h.numErrors = 0;

	for (int i = 0; i < d.n; i++){
		if (i < threshold){					// on the left hand side of threshold
			if (d.e.y[i] < 0){ //counting #negs
				h.numErrors++;
				h.errorIndices.push_back(i);
			}
		}
		else{								//on the right hand side of threshold
			if (d.e.y[i] > 0){ //counting #pos
				h.numErrors++;
				h.errorIndices.push_back(i);
			}
		}
	}
	return h;
}

hypothesis greaterThanHypothesis(data d, int threshold){// '>' of threshold
	hypothesis	h;
	h.thresholdIndex = threshold;
	h.sign = 0;// 1 = '<', 0 = '>'
	h.numErrors = 0;

	for (int i = 0; i < d.n; i++){
		if (i < threshold){					// on the left hand side of threshold
			if (d.e.y[i] > 0){ //counting #pos
				h.numErrors++;
				h.errorIndices.push_back(i);
			}
		}
		else{								//on the right hand side of threshold
			if (d.e.y[i] < 0){ //counting #neg
				h.numErrors++;
				h.errorIndices.push_back(i);
			}
		}
	}
	return h;
}

std::vector<int> findThresholds(std::vector<int> y){//Find candidate thresholds
	int					temp; //container to compare when sign of y changes
	std::vector<int>	thresholds; // list of indices where the sign of y has just changed

	//std::cout << "Candidate Thresholds: ";
	temp = y[0];
	for (int i = 0; i < y.size(); i++){
		if (y[i] != temp){
			thresholds.push_back(i);
			temp = y[i];
			//std::cout << i <<' ';
		}
	}
	//std::cout << endl;
	return thresholds;
};

data readDataset(std::string filename){
	vector<std::string>			tokenized_string;
	std::string					line;
	ifstream					inputStream;
	data						data;

	inputStream.open(filename);
	if (inputStream.is_open()){
		getline(inputStream, line); //Get T and n values
		tokenized_string = split(line, ' ');
		istringstream(tokenized_string[0]) >> data.T;
		istringstream(tokenized_string[1]) >> data.n;

		getline(inputStream, line); // getting x
		tokenized_string = split(line, ' ');
		for (int i = 0; i < tokenized_string.size(); i++){ //looping through x
			data.e.x.push_back(atof(tokenized_string[i].c_str()));
		}
		getline(inputStream, line); // getting y
		tokenized_string = split(line, ' ');
		for (int i = 0; i < tokenized_string.size(); i++){ //looping through y
			data.e.y.push_back(atoi(tokenized_string[i].c_str()));
		}
		getline(inputStream, line); // getting p
		tokenized_string = split(line, ' ');
		for (int i = 0; i < tokenized_string.size(); i++){ //looping through p
			data.e.p.push_back(atof(tokenized_string[i].c_str()));
		}
	}
	return data;
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