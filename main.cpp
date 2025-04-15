/*
Ali Husain
Dr. Rincon
COSC 3360: Programming Assignment 3
9 Apr 2025
*/

#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <utility>
#include <sstream>

struct lines { // defining lines struct from data given by input
    std::vector<std::pair<char, std::vector<std::pair<int, int> > > > ranges;
    int headPos[2]; 
    std::vector<int> dataPos;
    int index;
    char** outputMatrix; // make changes to matrix when done with figuring out character placement

    int *turn;
    pthread_mutex_t *printMutex;
    pthread_mutex_t *dataMutex;
    pthread_cond_t *condition;
};

void * charArt(void *arg) { // thread function, based off of Dr. Rincons threading practices
    lines imageLine = (*(lines *)arg); // fill Data for the thread to get ready for encoding algorithm
    std::vector<std::pair<char, std::vector<std::pair<int, int> > > > ranges = imageLine.ranges;
    int headPos[2] = {imageLine.headPos[0], imageLine.headPos[1]};
    std::vector<int> dataPos = imageLine.dataPos;
    int index = imageLine.index;
    char** outputMatrix = imageLine.outputMatrix;
    pthread_mutex_unlock(imageLine.dataMutex);
 
    for (int i = headPos[0]; i < headPos[1]; i++) { // loops through the range, algorithm for encoding 
        int data = dataPos[i];
        for (int j = 0; j < ranges.size(); j++) {
            char character = ranges[j].first;
            const std::vector<std::pair<int, int> >& inner_ranges = ranges[j].second;
            for (int k = 0; k < inner_ranges.size(); k++) {
                int start = inner_ranges[k].first; // gets the ranges from the nested vector pair
                int end = inner_ranges[k].second;
                if (data >= start && data <= end) { // checks to see if data falls in range if true adds char to the matrix
                    outputMatrix[index][data] = character;
                    break;
                } 
            }
        }
    }
    
    
    pthread_mutex_lock(imageLine.printMutex); // critical section for telling all threads to sleep when it is not their turn
    while (*imageLine.turn != imageLine.index) {
        pthread_cond_wait(imageLine.condition, imageLine.printMutex);
    }
    pthread_mutex_unlock(imageLine.printMutex);
    
    std::cout<< outputMatrix[index] << std::endl; // print the correct row via the index
    
    pthread_mutex_lock(imageLine.printMutex); // critical section to increment turn and wake up other threads for their turn
    (*imageLine.turn)+=1;
    pthread_cond_broadcast(imageLine.condition);
    pthread_mutex_unlock(imageLine.printMutex);

    return NULL;
 }

int main() {
    pthread_mutex_t printMutex; // initalize pthread mutexes and condition
    pthread_mutex_t dataMutex;
    pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
    pthread_mutex_init(&printMutex, NULL);
    pthread_mutex_init(&dataMutex, NULL);
    static int turn = 0;

    int col, row; // reading matrix size from first line
    std::cin >> col >> row;
    std::cin.ignore();
    char** outputMatrix = new char*[row];
    
    for (int r = 0; r < row; ++r) { // initalizing 2d array
        outputMatrix[r] = new char[col];
        std::fill(outputMatrix[r], outputMatrix[r] + col, ' '); // fill 2d array with spaces before decoding
    }
    
    std::vector<std::pair<char, std::vector<std::pair<int, int> > > > ranges; // vector of chars and vector pairs of ints (the ranges)
    std::string line;
    std::getline(std::cin, line); // reading second line of input to find the ranges of the chars, stored in nested vector pair thanks to multiple ranges
    std::istringstream iss(line);
    std::string token;
    while (std::getline(iss, token, ',')) {
        char id = token[0];
        std::vector<std::pair<int, int> > pairs;
        std::istringstream subIss(token.substr(2));
        int a, b;
        while (subIss >> a >> b) {
            pairs.emplace_back(a, b);
        }
        ranges.emplace_back(id, pairs);
    }

    std::vector<int> headPos; // get all of the head pos from the third line of input
    std::getline(std::cin, line);
    std::istringstream headStream(line);
    int value;
    while (headStream >> value) {
        headPos.push_back(value); // head pos values stored in vector
    }

    std::vector<int> dataPos; // get all of the data pos from the fourth line of input
    std::getline(std::cin, line);
    std::istringstream dataStream(line);
    while (dataStream >> value) {
        dataPos.push_back(value); // data pos values stored in vector
    }

    pthread_t *tid = new pthread_t[row]; // dynamic array of pthread_t of size of the image height (rows), based off of Dr. Rincon's threading practices
    lines imageLine;
    imageLine.turn = &turn;
    imageLine.printMutex = &printMutex;
    imageLine.dataMutex = &dataMutex;
    imageLine.condition = &condition;
    for (int i = 0; i < row; i++) { // for loop that iterates through the lines in the image (amount of rows)
        pthread_mutex_lock(&dataMutex);  // lock muex in here and initiliaze input in here
        imageLine.ranges = ranges; // put ranges in each member
        imageLine.headPos[0] = headPos[i]; // filling in head pos
        imageLine.headPos[1] = (i == row - 1) ? dataPos.size() : headPos[i+1];
        imageLine.dataPos = dataPos; // set to dataPosition
        imageLine.index = i;
        imageLine.outputMatrix = outputMatrix;
        if(pthread_create(&tid[i],nullptr,charArt,(void *) &imageLine)) { // create threads based off the imageLine
			std::cerr << "Error creating thread" << std::endl;
			return 1;
		}
    }

    for (int j = 0; j < row; j++) { // Waits for other threads to finish then call pthread_join 
        pthread_join(tid[j],nullptr);
    }

    for (int r = 0; r < row; ++r) { // deallocating memory
        delete[] outputMatrix[r];
    }
    delete[] outputMatrix; // remove
    delete[] tid;

    return 0;
}