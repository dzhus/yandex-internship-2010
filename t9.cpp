#include <iostream>
#include <string>
#include <algorithm>
#include <list>
#include <vector>

using namespace std;

typedef unsigned short int small_int;

const int bufsize = 100000;

/// Mapping of characters to digital keys
string char_keys[26] = {"2", "2", "2",
                        "3", "3", "3",
                        "4", "4", "4",
                        "5", "5", "5",
                        "6", "6", "6",
                        "7", "7", "7", "7",
                        "8", "8", "8",
                        "9", "9", "9", "9"};

/// Return digit corresponding to given character
const string& get_key(const char& c)
{
    return char_keys[c - 'a'];
}

/// Return full key (a string of digits from 2 to 9) for any
/// given word
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

    /// Word frequency must be increased by 1 after each use only if
    /// this is true
    bool bumpable;

public:
    Word()
    {}
    
    Word(const string &s, const small_int &freq, const bool &b = true)
        :str(s), frequency(freq), bumpable(b)
    {}

    void send(ostream &out)
    {
        out << str;
    }

    void bump(void)
    {
        frequency++;
    }
    
    const bool& is_bumpable(void) const
    {
        return bumpable;
    }

    const small_int& get_frequency(void) const
    {
        return frequency;
    }
};

ostream& operator <<(ostream &out, Word &w)
{
    w.send(out);
    return out;
}

/// If front is true, predicate is true for words with lower (or
/// equal) frequency. Otherwise, predicate gives truth if words have
/// higher frequency.
class WordFreqPred
{
private:
    small_int frequency;
    bool front;
public:
    WordFreqPred(int freq, bool f)
        :frequency(freq), front(f)
    {}

    bool operator() (Word &w) const
    {
        return (w.get_frequency() <= (frequency - (!front ? 1 : 0)));
    }
};

/// Insert new word into word list wrt frequency
///
/// If bump is false, word is inserted after all words with the same
/// frequency. Otherwise, word is put in front of all equifrequent
/// words.
list<Word>::iterator insert_word(list<Word> &l, const Word &w, bool bump = false)
{
    list<Word>::iterator i;
    i = find_if(l.begin(), l.end(), WordFreqPred(w.get_frequency(), bump));
    return l.insert(i, w);
}

class Trie
{
private:
    /// Words stored in root leaf of trie
    list<Word> words;

    /// Children tries
    vector<Trie*> children;
    
    /// Add word object under given full key
    void add_word_proc(string &full_key, const Word &w)
    {
        vector<Trie*>::size_type key = (full_key[0] - '1');
        if (!full_key.length())
        {
            insert_word(words, w);
        }
        else
        {
            full_key.erase(0, 1);
            if (children[key] == NULL)
                children[key] = new Trie();
            children[key]->add_word_proc(full_key, w);
        }
    }

    /// Get list of words stored in trie under given full key. We
    /// assume that all used words are present in the trie, so this
    /// always succeeds.
    list<Word>& get_leaf(string &full_key)
    {
        vector<Trie*>::size_type key = (full_key[0] - '1');
        if (!full_key.length())
            return words;
        else
        {
            full_key.erase(0, 1);
            return children[key]->get_leaf(full_key);
        }
    }
public:
    Trie(void)
    {
        /// Preallocate vector for 9 (from 1 to 9) children which may
        /// be added later
        children.resize(9, NULL);
    }

    ~Trie(void)
    {
        for (vector<Trie*>::iterator i = children.begin(); i != children.end(); i++)
            if (*i != NULL)
                delete *i;
    }

    /// Public wrapper for add_word_proc
    void add_word(const string &contents, const small_int &freq)
    {
        string fk = get_full_key(contents);
        add_word_proc(fk, Word(contents, freq));
    }

    /// Add new punctuation mark under 1
    void add_punctuation(const string &punct)
    {
        string fk = "1";
        add_word_proc(fk, Word(punct, 1, false));
    }

    /// Get n-th word stored in trie under given full key.
    const Word& query(string &full_key, int n = 0)
    {
        list<Word>& leaf = get_leaf(full_key);
        /// @internal We assume that leaf.length() > n
        list<Word>::iterator i = leaf.begin();
        int j = 0;
        while (j < n)
            i++, j++;

        /// Bump frequency and reinsert word if needed
        if ((*i).is_bumpable())
        {
            Word w = Word(*i);
            w.bump();
            leaf.erase(i);
            i = insert_word(leaf, w, true);
        }

        return *i;
    }
};

class T9Reader
{
private:
    string full_key;
    bool prev_space, word_put;
    int skips;
    Word trie_word;
    Trie *trie;
public:
    T9Reader(Trie *t)
    {
        trie = t;
        full_key = "";
        word_put = false;
        prev_space = false;
        skips = 0;
    }

    /// Print word selected so far
    void put_current_word(void)
    {
        trie_word = trie->query(full_key, skips);
        cout << trie_word;
        full_key.clear();
        skips = 0;
        word_put = true;
    }

    void read(const string &input)
    {
        for (string::const_iterator i = input.begin(); i != input.end(); i++)
        {
            if (*i == ' ')
            {
                if (!prev_space)
                    put_current_word();
                prev_space = true;
                cout << ' ';
            }
            else
            {
                if ((*i >= '2') && (*i <= '9'))
                {
                    full_key += *i;
                    word_put = false;
                }
                else if (*i == '*')
                    skips++;
                else if (*i == '1')
                {
                    if (!prev_space)
                        put_current_word();
                    full_key = "1";
                    word_put = false;
                }
                prev_space = false;
            }
        }
        if (!word_put)
            put_current_word();
    }
};

int main(int argc, char* argv[])
{
    int dict_size;
    string dict_word;
    small_int freq;
    Trie tr;
    T9Reader t9 = T9Reader(&tr);
    char buf[bufsize];

    cin >> dict_size;

    tr.add_punctuation(".");
    tr.add_punctuation(",");
    tr.add_punctuation("?");

    /// Populate trie
    for (int i = 0; i < dict_size; i++)
    {
        cin >> dict_word >> freq;
        tr.add_word(dict_word, freq);
    }

    cin.ignore(1);
    cin.getline(buf, bufsize);
    t9.read(buf);
    
    return 0;
}
