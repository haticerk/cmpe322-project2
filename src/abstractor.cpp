/*
* @file abstractor.cpp
* @author Hatice Erk 
*
* @brief Code for CMPE322 project2 to create multithreaded search engine that queries the paper abstracts and summaries the most relevant ones.
* 
* How to compile and run:
* 	g++ -o abstractor.out abstractor.cpp -lpthread
*	./abstractor.out <input_file> <output_file>
*/

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include <vector> 
#include <algorithm>
#include <iomanip>

using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
string names[26] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};

/* 
* The struct FileCheck are designed to encounter all file releated jobs.
* 
* filename: 	the name od file, takes from input file
* checked: 		initially false; when a thread started work on file, it changes to true
* similarity:	the Jaccard similarity between file and query, writes after thread calculated
* summary:		the summary of file
*/
struct FileCheck{
	string filename;
	bool checked = false;
	float similarity=0;
	vector<string> summary;
};

vector<FileCheck> files;
vector<string> query;

/*
* The function that threads executes
*
* param: the name of thread
*/
void *abstract(void *param){
	string thread_name = *(string *) param;

	for(int i=0; i<files.size(); i++){
		if(files[i].checked == false){	//for every file threads check if the file checked before

			pthread_mutex_lock(&mutex);
     		files[i].checked = true;	//updates checked value, make that in mutex to avoid synchronization issues
    		pthread_mutex_unlock(&mutex);
    		
    		if(files[i].checked == true){
    			//first, writes file that which thread working on which file
    			ofstream outFile;
     			outFile.open(query[0],ios::app);
				outFile << "Thread " << thread_name << " is calculating " << files[i].filename << endl;
    			
    			cout << "Thread " << thread_name << " is calculating " << files[i].filename << endl;
				//opens releated file from "abstrats" directory
    			string path = "../abstracts/"+ files[i].filename;
    			ifstream infile(path);
    			
    			//makes all words a vector and then also makes a vector for word set  
    			vector<string> allwords;
    			vector<string> words;
    			string lines;
				while(getline(infile,lines)){
					stringstream ss(lines);
					string str;
					while(ss >> str){
						allwords.push_back(str);
						if(!count(words.begin(),words.end(),str)){
							words.push_back(str);
						}
					}
				}

				//compares query and the word set
				int intersection = 0;
				for(int k=0; k<words.size(); k++){
					for(int j=1; j<query.size(); j++){
						if(words[k]==query[j]){
							intersection +=1;
						}
					}
				}
				//calculates first the union than the similarity
				int uni = 0;
				uni = words.size() + query.size() - 1 - intersection;
				float sim = (float)intersection / (float)uni;	
				files[i].similarity = sim;

				//for the summary, keeps all dots index
				vector<int> dots;
				dots.push_back(-1);
				for(int k=0; k<allwords.size(); k++){
					if(allwords[k]=="."){
						dots.push_back(k);
					}
				}

				for(int h=0; h<dots.size(); h++){
					cout << dots[h] << endl;
				}
			
				//check every sentence if it contains any word from query.
				for(int e=0; e< dots.size()-1;e++){
					for(int j=dots[e]; j<dots[e+1]; j++){
						for(int k=1; k<query.size(); k++){	
							if(allwords[j]==query[k]){	
								for(int m=dots[e]+1; m<dots[e+1]+1; m++){
									files[i].summary.push_back(allwords[m]);
									allwords[m]=""; 
								}	
							}
						}
					} 	
				}
			}
		}
	}

	pthread_exit(NULL);
}

/*
* Takes two filecheck struct, then compares them in order to their similarity value.
*/
bool compareResults(FileCheck f1, FileCheck f2){
	return (f1.similarity > f2.similarity);
}

/*
* The main function
* 
* takes two arguments:
* <input_file_name> <output_file_name> 
*/
int main(int argc, char *argv[]) {
	ifstream infile(argv[1]);
	string outputFile = argv[2];

	ofstream output;
	output.open(outputFile);
	output.close();
	
	//pushs the name of output file to query for threads can write to that
	query.push_back(outputFile);

	//reading from input file
	int T, A, N;
	string firstline; 
	getline(infile, firstline);
	istringstream iss(firstline); 
	iss >> T >> A >> N;

	string secondline;

	getline(infile,secondline);
	stringstream ss(secondline);

	string str;
	while(ss >> str){
		query.push_back(str);
	}
	
	string filename;
	while(getline(infile,filename)){
		FileCheck file;
		file.filename = filename; 
		files.push_back(file);
	}

	//creates the threads
	pthread_t threads[T]; 
	for(int i = 0; i < T; i++){
		pthread_create(&threads[i], NULL, abstract, &names[T-1-i]);
		cout << names[T-1-i] << endl;
	}

	//waits for every thread to finish
	for(int i = 0; i < T; i++){
		pthread_join(threads[i], NULL);
	}
	
	sort(files.begin(), files.end(), compareResults);
	
	//writes to output file
	output.open(outputFile, ios::app);
	output << "###" << endl;
	for(int i=0; i<N; i++){
		output << "Result " << i+1 << ":" << endl;
		output << "File: " << files[i].filename << endl;
		output << "Score: " <<setprecision(4) << fixed << files[i].similarity << endl;
		output << "Summary: ";
		for(int k=0; k<files[i].summary.size(); k++){
			output << files[i].summary[k] << " " ;
		}
		output << endl;
		output << "###" << endl;
	}

	pthread_exit(NULL);
    return 0;
}