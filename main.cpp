//#include <boost/algorithm/string/replace.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <future>
#include <algorithm>


using namespace std;
//g++ read.cpp -pthread -std=c++11

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

void printMap(const map<string, int> &m) {
    for (auto elem : m) {
        cout << elem.first << " : " << elem.second << "\n";
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

void write_to_file(const map<string, int> &m, string path) {
    ofstream myfile;
    myfile.open(path);
    for (auto elem : m) {
        myfile << elem.first << "    " << elem.second << "\n";
    }
    myfile.close();
}

map<string, int> mapper(int l, int r, const vector<string> &words) {
    map<string, int> mp;
    for (; l <= r; ++l) {
        ++mp[words[l]];
    }
    return mp;

}

void reducer( map<string, int> &master, const map<string, int>& mp){
    for (auto w: mp) {
        master[w.first] += w.second;
    }
}

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


int main(int argc, char *argv[]) {  // input_file, threads, output_file

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


    vector<string> words;
    map<string, int> m;
    vector< future<map<string, int>> > result_futures;

    words = open_read(infile);

    vector<int> list_of_words_amount = SplitVector(words, threads_n);

    auto start = get_current_time_fenced();
    for (int a = 0; a < list_of_words_amount.size()-1; ++a) {

//        cout << (i + 1) << " interval from " << a << " to " << b - 1 << " word" << endl;
        result_futures.push_back(
                async(std::launch::async, mapper,  list_of_words_amount[a],  list_of_words_amount[a+1] - 1, cref(words))
        );
    }

    vector<map<string, int>> results;
    for(size_t i = 0; i<result_futures.size(); ++i)
    {
        reducer(m, result_futures[i].get());
    }
    auto finish = get_current_time_fenced();
    auto total = finish - start;
    cout << "Time: " << to_us(total) << endl;




    write_to_file(m, "result.txt");
    printMap(m);
    return 0;
}

