#include <iostream>
#include <fstream>
#include <cstddef>
#include <list>
#include <vector>
#include <map>

using namespace std;

class Uzel{
public:
	char s;
	int key;
	Uzel* left, * right;
	Uzel(char s = 0, int k = 0) {
		this->s = s;
		this->key = k;
		left = NULL;
		right = NULL;
	}
	Uzel(Uzel* l, Uzel* r) {
		this->key = l->key + r->key;
		this->left = l;
		this->right = r;
	}
	~Uzel() {
		delete[] left;
		delete[] right;
	}
};

bool cmp (const Uzel* a, const Uzel* b) {
    return a->key < b->key;
}


// рекурсивный обход и построение кодов
map<char, vector<bool>> recursion_left(vector<bool>& code, Uzel* head, map<char, vector<bool>>& table_code) {
    if (head->left != nullptr){
        code.push_back(0);
        recursion_left(code, head->left, table_code);
    }

    if (head->right != nullptr){
        code.push_back(1);
        recursion_left(code, head->right, table_code);
    }

    if (head->left == nullptr && head->right == nullptr){
        table_code[head->s] = code;
    }

    if (!code.empty()) code.pop_back();

    return table_code;
}

map<char, vector<bool>> fill_code_t(Uzel* head, map<char, vector<bool>>& table_code) {
    vector<bool> code;
    return recursion_left(code, head, table_code);
}

class Huffman{
private:
    map<char, int> table_freq; //таблица с частотами
    map<char, int>::iterator iter;
    list<Uzel*> list;          //дерево
    Uzel* root;
    map<char, int> count_freq_en(ifstream& file);
    map<char, int> count_freq_de(ifstream& file, int n);
    Uzel* creat_tree();
public:

    void encode(ifstream& file, ofstream& result);
    void decode(ifstream& file, ofstream& result);
};

void Huffman:: encode(ifstream& file, ofstream& result) {
    count_freq_en(file); //подсчитываем частоты символов
    creat_tree();

    map<char, vector<bool>> table_code;
    fill_code_t(root, table_code);

    int last_byte = 0;
    result << last_byte;
    result << "\t";


    result << table_freq.size();
    for (iter = table_freq.begin(); iter != table_freq.end(); ++iter){
        result << iter->first;
        result << iter->second;
    }
    file.clear();
    file.seekg(0);
                   
    //записываем текст в файл
    char s, t = 0;
    int byte = 0; 
    while ((s = file.get()) && !file.eof()){
        vector <bool> a = table_code.at(s);

        for (size_t i = 0; i < a.size(); i++){
            t = t | a[i] << (7 - byte);
            byte++;
            if (byte == 8){
                byte = 0;
                result << t;
                t = 0;
            }
        }
    }

    if (byte) result << t;

    result.clear();
    result.seekp(0);
    result << byte;
};

void Huffman:: decode(ifstream& file, ofstream& result){
    int last_byte;
    file >> last_byte;  

    int n;
    file >> n;
    count_freq_de(file, n);
    creat_tree();

    //записываем наш текст в раскодированный файл
    char t = file.get();
    Uzel* head = root;
    int m = 0;
    while (!file.eof()) {
        bool bit = t & (1 << (7 - m));

        if (bit) head = head->right;
        if (!bit) head = head->left;

        if (head->left == nullptr && head->right == nullptr){
            result << head->s;
            head = root;
        }
        m++;

        if (m == 8){
            m = 0;
            t = file.get();
            if (last_byte == 0) continue;
            if (file.peek() == EOF){
                while (m != last_byte){
                    bit = t & (1 << (7 - n));
                    if (bit) head = head->right;
                    if (!bit) head = head->left;

                    if (head->right == nullptr && head->left == nullptr){
                        result << head->s;
                        head = root;
                    }
                    m++;
                }
            }
        }
    }

};

Uzel* Huffman::creat_tree() {
    list.clear();
    for (iter = table_freq.begin(); iter != table_freq.end(); ++iter){
        Uzel* p = new Uzel(iter->first, iter->second);
        list.push_back(p);
    }
    while (list.size() != 1){
        list.sort(cmp);

        Uzel* a = list.front();
        list.pop_front();
        Uzel* b = list.front();
        list.pop_front();
        Uzel* ab = new Uzel(a, b);
        list.push_back(ab);
    }
    root = list.front();
    return root;
}; 

map<char, int> Huffman:: count_freq_en(ifstream& file) {
    table_freq.clear();

    char s;
    while ((s = file.get()) && !file.eof()){
        table_freq[s]++;
    }
    return table_freq;
};

map<char, int> Huffman:: count_freq_de(ifstream& file, int n) {
    char s;
    int freq;
    table_freq.clear();
    while (n > 0){
        s = file.get();
        file >> freq;
        n--;
        table_freq[s] = freq;
    }
    return table_freq;
};




int main()
{
    ifstream file("input.txt");
    if (!file) return 1;

    ofstream result("output.txt", ios::out | ios::binary);
    if (!result) return 1;
    
    Huffman hf;
    hf.encode(file, result);

    file.close();
    result.close();

    ifstream file2("output.txt", ios::out | ios::binary);
    if (!file2) return 1;

    ofstream result2("result.txt");
    if (!result2) return 1;

    hf.decode(file2, result2);

    file.close();
    result.close();

    ifstream fin1("input.txt", ios::binary);
    ifstream fin2("result.txt", ios::binary);
    if (fin1 && fin2){
        char ch1, ch2;
        bool result = true;
        while (fin1.get(ch1) && fin2.get(ch2)){
            if (ch1 != ch2){
                result = false;
                break;
            }
        }
        if (result) cout << "Equal" << endl;
        else cout << "Unequal" << endl;
    }
    else cout << "Error opening file!" << endl;

    return 0;
}
