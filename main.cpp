#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <thread>
#include <set>
#include <algorithm>
#include <iterator>
#include <mutex>
#include <future>
#include <sstream>

using namespace std;


inline std::chrono::high_resolution_clock::time_point get_current_time_fenced() {
    std::atomic_thread_fence(memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us(const D& d) {
    return std::chrono::duration_cast<chrono::microseconds>(d).count();
}

void printMap(const map<string,int> &m) {
    for (auto elem : m) {
        cout << elem.first << ":" << elem.second << "\n";
    }
}

void write_to_file(const map<string,int>  &m, string path) {
    ofstream myfile;
    myfile.open(path);
    for (auto elem : m) {
        myfile << elem.first << "    " << elem.second << "\n";
    }
    myfile.close();
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
        for (size_t i = 0, len = word.size(); i < len; i++)
        {
            auto to = begin(word);
            for (auto from : word)
                if (!ispunct(from))
                    *to++ = from;
            word.resize(distance(begin(word), to));
        }

        transform(word.begin(), word.end(), word.begin(), ::tolower);
        words.push_back(word);
    }
    myfile.close();
    return words;
}



map<string, int> CountWords(const vector<string>& wordsVector, int start, int end) {
    map<string, int> mp;
    for (; start <= end; ++start) {
        ++mp[wordsVector[start]];
    }
    return mp;
}
void reducer( map<string, int> &wordsCount, const map<string, int>& local_map){
    for (auto w: local_map) {
        wordsCount[w.first] += w.second;
    }
}


// Divide vector into n equally parts, n - number of threads
vector<int> SplitVector(const vector<string>& vec, int n) {

    vector<int> outVec;
    int length = int(vec.size())/ n;
    int count = 0;
    int sum = 0;

    outVec.push_back(0);
    while(count != n - 1){
        sum += length;
        outVec.push_back(sum);
        //cout<<outVec[count]<<endl;
        count++;
    }
    outVec.push_back(int(vec.size()));
    return outVec;
}


int main() {

    string input_data[4], infile, out_by_a, out_by_n;
    int threads_n;
    ifstream myFile;
    myFile.open("data_input.txt");

    for(int i = 0; i < 4; i++)
        myFile >> input_data[i];
    myFile.close();

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < input_data[i].length(); j++) {
            if (input_data[i][j] == '=')
                input_data[i][j] = ' ';
        }
        stringstream ss(input_data[i]);
        string temp;
        int k = 0;
        while (ss >> temp) {
            if (k != 0) {
                stringstream lineStream(temp);
                string s;
                getline(lineStream,s,',');
                s.erase(remove( s.begin(), s.end(), '\"' ), s.end());
                input_data[i] = s;
            }
            k++;
        }
    }

    infile = input_data[0];
    out_by_a = input_data[1];
    out_by_n = input_data[2];
    threads_n = stoi(input_data[3]);




    map<string, int> wordsCount;


    // Read file with text
    vector<string> vectorWords;
    vectorWords = open_read("data.txt");


    vector<int> list_of_words_amount = SplitVector(vectorWords, threads_n);


    vector<future<map<string, int>>> futures;
    auto start = get_current_time_fenced();
    for (int i = 0; i < list_of_words_amount.size() - 1; ++i) {
        futures.push_back(async(CountWords, cref(vectorWords), list_of_words_amount[i], list_of_words_amount[i + 1]));
    }
    for(size_t i = 0; i<futures.size(); ++i)
    {
        reducer(wordsCount, futures[i].get());
    }
    auto finish = get_current_time_fenced();
    auto total = finish - start;
    cout << "Time: " << to_us(total) << endl;
    printMap(wordsCount);
    write_to_file(wordsCount, "result.txt");

}






