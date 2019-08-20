#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <unordered_map>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdio>
using namespace std;

#define INF unsigned(-1)

const unsigned long long SEC = (unsigned long long) 1000000;
const unsigned long long MIN = (unsigned long long) 60 * SEC;
const unsigned long long HOUR = (unsigned long long) 60 * MIN;
const unsigned long long DAY = (unsigned long long) 24 * HOUR;

int NUM_THREADS = 8;
int max_test_words = 3500;
int total_layouts = 0;
FILE *fp_output, *fp_words, *fp_layouts;

string qwerty = "qwertyuiopasdfghjklzxcvbnm"; 
vector<string> word_list;
// the new thing
unordered_map<string, double> top_list;

double ** user_list;

// double user_list[10][26];

vector<string> *qwertyHandArray;

struct ArgList {
	int idx;
	vector<string> * layout;
};
ArgList * args;


pthread_t * threads;
pthread_mutex_t mutex_lock;

inline long long time_stamp() {
	struct timeval query_time;
	gettimeofday(&query_time, nullptr);
	return query_time.tv_sec * 1000000 + query_time.tv_usec;
}

inline int mapLetter(char c) {
	switch(c) {
	case 'q':
		return 0;
	case 'w':
		return 1;
	case 'e':
		return 2;
	case 'r':
		return 3;
	case 't':
		return 4;
	case 'y':
		return 5;
	case 'u':
		return 6;
	case 'i':
		return 7;
	case 'o':
		return 8;
	case 'p':
		return 9;
	case 'a':
		return 10;
	case 's':
		return 11;
	case 'd':
		return 12;
	case 'f':
		return 13;
	case 'g':
		return 14;
	case 'h':
		return 15;
	case 'j':
		return 16;
	case 'k':
		return 17;
	case 'l':
		return 18;
	case 'z':
		return 19;
	case 'x':
		return 20;
	case 'c':
		return 21;
	case 'v':
		return 22;
	case 'b':
		return 23;
	case 'n':
		return 24;
	case 'm':
		return 25;
	default:
		return -1;
	}
}

// read the max word(3500) from the txt file 
void readWords(const char * filename) {
	ifstream fin(filename);
	string line;
	// new 
	string numS;
	int count = 0;
	while (getline(fin, line)) {
		if (count >= max_test_words) {
			break;
		}
		int i;
		for (i = 0; i < line.length(); i++) {	
			if (line[i] == ',') {
				break;
			}
		}
		numS = line.substr(i+1);
		double num = stod(numS) * 100;
		word_list.push_back(line.substr(0, i));
		top_list[line.substr(0, i)] = num;
		count++;
	}
	fin.close();
}

// read the layout from the txt file
void readLayout(const char * filename) {
	ifstream fin(filename);
	string line;
	int thread_idx = 0;
	total_layouts = 0;
	while (getline(fin, line)) {
		qwertyHandArray[thread_idx].push_back(line);
		total_layouts++;
		if (++thread_idx == NUM_THREADS) {
			thread_idx = 0;
		}
	}
	fin.close();
}

//user_list[number] = 
void readUser(int number) {
	string dir = "U" + to_string(number+1) + "_dic_data.txt";
	ifstream fin(dir);
	string line;
	// new 
	string numS;
	while (getline(fin, line)) 
	{
		int i;
		for (i = 0; i < line.length(); i++) 
		{	
			if (line[i] == ' ') 
			{
				break;
			}
		}
		string part = line.substr(0, i);
		char letter = part[0];
		// cout << "letter: " << letter << endl;
		numS = line.substr(i+1);
		double num = stod(numS);
		user_list[number][letter-'a'] = num;
	}
	fin.close();
}

// get the Hand and Words mapping and save it 
double getHandAndWord(string* qwertyHandArray, double* user_list) {
	double total = 0;
	// srand(time(NULL));
	for (string& word : word_list) {
		// cout << "word:" << word <<endl;
		int count = 0;
		for (char c : word) {
			char user_hand;
			char q_hand = (*qwertyHandArray)[mapLetter(c)];
			// double ran_num = ((double) rand()) / RAND_MAX;
			// srand(time(0));
			double ran_num = rand() % 1000 / (double)1001;
			// cout <<"word: " << word << "ran_num: " << ran_num << endl;
			if (ran_num < user_list[c-'a']) {
				// cout <<"word: " << word << "user_num: " << user_list[c-'a'] << endl;
				user_hand = '0';
			} else {
				user_hand = '1';
			}
			if (user_hand == q_hand) {
				// cout << "user_hand: " << user_hand << "q_hand: " << q_hand << endl;
				count = count + 1;
				if (count == word.length()) {
					// cout << "word: "<< word <<"length: " << word.length() << endl;
					total += top_list[word];
				}
			} else {
				break; // prune v.剪枝
			}
		}
	}
	return total;
}

int binaryToInt(string bin)
{
    int n = 0;
    for (int i = 0; i < 26; i++)
    {
        n = n * 2 + (bin[i] - '0');
    }
    return (int)n;
}

// the multithreading method
void *CalKeyboard(void *threadL){
	// cout << "Got into CalKeyboard" << endl;
	ArgList * tmp = (ArgList *) threadL;

	int idx = tmp->idx;
	vector<string> * QHR = tmp->layout;

	for (string & lout : *QHR) {
		string * converted = &lout;

		for (int i =0; i < 10; i++){
		
		// run simulation
		double ans = getHandAndWord(converted, user_list[i]);

		string ss = converted->data();
		// binaryToInt(ss);
		int out = binaryToInt(ss);
		// cout << "out:" << out << "actual" << ss  << endl;


		pthread_mutex_lock(&mutex_lock);
		// fprintf(fp_output, "%s %f %d\n", converted->data(), ans, i);
		fprintf(fp_output, "%d %f %d\n", out, ans, i);
		pthread_mutex_unlock(&mutex_lock);
	}
}
	pthread_exit(NULL);

}

void init() {
	qwertyHandArray = new vector<string>[NUM_THREADS];
	args = new ArgList[NUM_THREADS];   
	threads = new pthread_t[NUM_THREADS];

	user_list = new double * [10];
	for (int i =0; i < 10; i++){
			user_list[i] = new double[26];
			readUser(i);
	}
}


int main(int argc, char const *argv[])
{

	if (argc < 4) {
		printf("Hand overlap simulation program.\n\n");
		printf("Usage:\n");
		printf("    %s <words_file> <layout_file> <output_file> [<thread_num = %d>]\n\n", argv[0], NUM_THREADS);
		return 0;
	}

	if (argc >= 5) {
		NUM_THREADS = atoi(argv[4]);
	}

	printf("Number of threads: %d\n", NUM_THREADS);
	init(); // needs NUM_THREADS parameter

	printf("Reading words file:   \"%s\"\n", argv[1]);
	readWords(argv[1]);
	printf("Reading layouts file: \"%s\"\n", argv[2]);
	readLayout(argv[2]);
	printf("Words in total:   %d\n", max_test_words);
	printf("Layouts in total: %d\n", total_layouts);

	fp_output = fopen(argv[3], "w");
	printf("Writing to file:      \"%s\"\n", argv[3]);

	// readUser("U5_dic_data.txt");

	printf("Simulation begins\n");
	long long start = time_stamp();

	pthread_mutex_init(&mutex_lock, NULL);

	//initilize and set thread joinable
	for(int i = 0; i < NUM_THREADS; i++) {
		// cout << "main(): creating threads," << i << endl;
		struct timeval time; 
    	gettimeofday(&time,NULL);
    	srand((time.tv_sec * 1000) + (time.tv_usec / 1000));
		args[i].idx = i;
		args[i].layout = &qwertyHandArray[i];
		int rc = pthread_create(&threads[i], NULL, CalKeyboard, (void *) &args[i]);
		if (rc) {
			cout << "Error: unable to create thread," << rc << endl;
			exit(-1);
		}
	}

	for (int i = 0; i< NUM_THREADS; i++) {
		int rc = pthread_join(threads[i], NULL);
		if (rc) {
			cout << "Error: unable to join," << rc << endl;
			exit(-1);
		}
		// cout << "main: completed thread id :" << i << endl;
	}

	long long end = time_stamp();
	fclose(fp_output);
	printf("Simulation ends\n");

	// double total = (end - start) / (double)1000000;
	unsigned long long total = end - start;
	unsigned long long days = total / DAY;
	total %= DAY;
	unsigned long long hours = total / HOUR;
	total %= HOUR;
	unsigned long long minutes = total / MIN;
	total %= MIN;
	unsigned long long seconds = total / SEC;
	printf("Simulation time:  %llu day %llu hour %llu min %llu sec\n", days, hours, minutes, seconds);

	// cout << "main(): program exiting." << endl;
}