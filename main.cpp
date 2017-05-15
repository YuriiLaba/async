//#include <boost/algorithm/string/replace.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <future>


using namespace std;
//g++ read.cpp -pthread -std=c++11
using words_counter_t = map<string, int>;
words_counter_t m;
mutex myMutex;
//words_counter_t checkM;

//string format_word(string word) {
//    boost::replace_all(word, "-", " ");
//    boost::replace_all(word, "`", " ");
//    boost::replace_all(word, "?", " ");
//    boost::replace_all(word, ".", " ");
//    boost::replace_all(word, "!", " ");
//    boost::replace_all(word, "\"", " ");
//    boost::replace_all(word, ".", " ");
//    boost::replace_all(word, "'", " ");
//    boost::replace_all(word, "'", " ");
//    boost::replace_all(word, ";", " ");
//    boost::replace_all(word, ",", " ");
//    return word;
//}

void printMap(const words_counter_t &m) {
    for (auto elem : m) {
        cout << elem.first << " " << elem.second << "\n";
    }
}


vector<string> open_read(string path) {
    ifstream myfile;
    vector<string> words;
    string word;
    myfile.open(path);
    if (!myfile.is_open()) {
        cerr << "Error" << endl;
        return words;
    }
    string formated_word;
    while (myfile >> word) {
        formated_word = word;  //format_word(word);
        words.push_back(formated_word);
        //        ++checkM[word];       // check map with only main thread
    }
    myfile.close();
    return words;
}

void write_to_file(const words_counter_t &m, string path) {
    ofstream myfile;
    myfile.open(path);
    for (auto elem : m) {
        myfile << elem.first << "    " << elem.second << "\n";
    }
    myfile.close();
}

words_counter_t mapper(int l, int r, const vector<string> &words) {
    words_counter_t mp;
    for (; l <= r; ++l) {
        ++mp[words[l]];
    }
    return mp;

}

void reducer( words_counter_t &master, const words_counter_t& mp){
    for (auto w: mp) {
        master[w.first] += w.second;
    }
}


int main(int argc, char *argv[]) {  // input_file, threads, output_file
    vector<string> words;
    vector< future<words_counter_t> > result_futures;

    if (!argv[1]) words = open_read("data.txt");
    else words = open_read(argv[1]);


    istringstream ss(argv[2]);

    cout << "Spawning " << 5 << " workers" << endl;
    for (int i = 0; i < 5; ++i) {
        size_t a = (words.size()) / 5 * i;
        size_t b = (words.size()) / 5 * (i + 1);
//        cout << (i + 1) << " interval from " << a << " to " << b - 1 << " word" << endl;
        result_futures.push_back(
                async(std::launch::async, mapper, a, b - 1, cref(words))
        );
    }

    vector<words_counter_t> results;
    for(size_t i = 0; i<result_futures.size(); ++i)
    {
        reducer(m, result_futures[i].get());
    }


//    printMap(checkM);
//    cout << "==================================" << endl;

    write_to_file(m, argv[3]);
    printMap(m);
    return 0;
}

