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
    std::vector<std::pair<char, std::vector<std::pair<int, int> > > >& ranges;
    std::vector<int> headPos;
    std::vector<int>& dataPos;
    int index;
    char** outputMatrix; // make changes to matrix when done with figuring out character placement
};

void * asciiArt(void *void_ptr);

int main() {
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
    
    std::vector<lines> arg; // storing thread in lines vector, each contains refrences to dataPos, and the matrix while having its unique index + head pos
    arg.reserve(row);
    for (int i = 0; i < row; i++) {
        lines args = {ranges, std::vector<int>(), dataPos, i, outputMatrix};
        if (i == row - 1) { // if statment that essentially checks if its the last index, when true the range starts off on the last head pos to the last element of dataPos
            args.headPos.push_back(headPos[i]);
            args.headPos.push_back((int)dataPos.size());
        }
        else { 
            args.headPos.push_back(headPos[i]);
            args.headPos.push_back(headPos[i+1]);
        }
        arg.push_back(args);
    }

    pthread_t *tid = new pthread_t[row]; // dynamic array of pthread_t of size of the image height (rows), based off of Dr. Rincon's threading practices
    for (int i = 0; i < row; i++) { // for loop that iterates through the lines in the image (amount of rows)
        if(pthread_create(&tid[i],nullptr,asciiArt,(void *) &arg.at(i))!= 0) {
			std::cerr << "Error creating thread" << std::endl;
			return 1;
		}
    }

    for (int j = 0; j < row; j++) { // Waits for other threads to finish then call pthread_join 
        pthread_join(tid[j],nullptr);
    }

    for (int r = 0; r < row; r++) { // printing out the matrix 
        for (int c = 0; c < col; c++) {
            std::cout << outputMatrix[r][c];
        }
        std::cout << std::endl;
    }

    for (int r = 0; r < row; ++r) { // deallocating memory
        delete[] outputMatrix[r];
    }
    delete[] outputMatrix;
    delete[] tid;

    return 0;
}

void * asciiArt(void *void_ptr) { // thread function, based off of Dr. Rincons threading practices
    lines *ptr = (lines *) void_ptr;
    std::vector<std::pair<char, std::vector<std::pair<int, int> > > >& ranges = ptr->ranges;
    std::vector<int>& dataPos = ptr->dataPos;
    int index = ptr->index;
    char** outputMatrix = ptr->outputMatrix;
 
    for (int i = ptr->headPos[0]; i < ptr->headPos[1]; i++) { // loops through the range
         int data = ptr->dataPos[i];
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
 
    return NULL;
 }