#include <iostream>
#include <string>
#include <list>
#include <vector>

using namespace std;

typedef unsigned short int small_int;

/// Mapping of characters to digital keys
string char_keys[26] = {"2", "2", "2",
                        "3", "3", "3",
                        "4", "4", "4",
                        "5", "5", "5",
                        "6", "6", "6",
                        "7", "7", "7", "7",
                        "8", "8", "8",
                        "9", "9", "9", "9"};

const string& get_key(const char& c)
{
    return char_keys[c - 'a'];
}

string get_full_key(const string& s)
{
    string res;
    
    for (string::const_iterator i = s.begin(); i != s.end(); i++)
        res += get_key(*i);

    return res;
}

/// Word with frequency
class Word
{
    string str;
    small_int frequency;
public:
    Word(string &s, small_int &freq)
    {
        str = s;
        frequency = freq;
    }
};

class Trie
{
private:
    list<Word> words;
    vector<Trie*> children;
public:
    Trie(void)
    {
        children.resize(9, NULL);
    }

    /// Add word contents with given full key and frequency
    void add_word(string &full_key, string &contents, small_int &freq)
    {
        int key = full_key[0];
        if (!full_key.length())
            words.push_back(Word(contents, freq));
        else
        {
            full_key.erase(0, 1);
            if (children[key] == NULL)
                children[key] = new Trie();
            children[key]->add_word(full_key, contents, freq);
        }
    }
};

int main(int argc, char* argv[])
{
    int dict_size;
    small_int freq;
    Trie tr;
    string word;

    cin >> dict_size;

    for (int i = 0; i < dict_size; i++)
    {
        cin >> word >> freq;
        cout << get_full_key(word) << endl;
    }

    return 0;
}
