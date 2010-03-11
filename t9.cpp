#include <iostream>
#include <string>
#include <algorithm>
#include <list>
#include <vector>
#include <stack>

using namespace std;

typedef unsigned short int small_int;

/// Trie level.
typedef unsigned char level_t;

/// Word frequency.
///
/// @internal Possible overflow at 65k
typedef small_int frequency_t;

const int bufsize = 100000;

/// Mapping of characters to digital keys
const char char_keys[26] = {'2', '2', '2',
                            '3', '3', '3',
                            '4', '4', '4',
                            '5', '5', '5',
                            '6', '6', '6',
                            '7', '7', '7', '7',
                            '8', '8', '8',
                            '9', '9', '9', '9'};

/// Word with frequency
class Word
{
public:
    const string str;

    /// Word frequency must be increased by 1 after each use only if
    /// this is true
    const bool bumpable;
    
    Word()
        :bumpable(false)
    {}

    Word(const string &s, const bool &b = true)
        :str(s), bumpable(b)
    {}

    void print() const
    {
        cout << str;
    }

    void send(ostream &out)
    {
        out << str;
    }
};

ostream& operator <<(ostream &out, Word &w)
{
    w.send(out);
    return out;
}

typedef list<Word> word_list;

/// Insert new word into word list wrt frequency
///
/// @param bump If true, word is inserted in the beginning of list.
///             Otherwise, it's appended after all other items.
///
/// @return List beginning iterator.
word_list::iterator insert_word(word_list &l, const Word &w, bool bump = false)
{
    if (bump)
        l.push_front(w);
    else
        l.push_back(w);
    return l.begin();
}

template <class Label, class Data>
class BinaryTree
{
private:


    /// Data stored in this node
    Data data;
    
    BinaryTree *prev, *next;
public:
    /// Node label
    const Label label;
    
    /// STL-style input in-order iterator for tree.
    class iterator {
    private:
        /// Standard traits.
        typedef input_iterator_tag iterator_category;
        typedef Data& value_type;
        typedef ptrdiff_t distance_type;

        /// Tree we iterate over
        BinaryTree *tree;

        /// Traversal stack
        stack<BinaryTree*> iter_stack;

    public:
        /// @param end If true, create end iterator
        iterator(BinaryTree *ptr, bool end = false)
            :tree(ptr)
        {
            if (!end)
                while (ptr != NULL)
                {
                    iter_stack.push(ptr);
                    ptr = ptr->prev;
                }
        }

        iterator& operator ++(int)
        {
            BinaryTree *ptr = iter_stack.top();
            iter_stack.pop();
            ptr = ptr->next;
            while (ptr != NULL)
            {
                iter_stack.push(ptr);
                ptr = ptr->prev;
            }
            return *this;
        }

        iterator& operator ++(void)
        {
            *this++;
            return *this;
        }

        bool operator==(iterator iter)
        {
            return (tree == iter.tree && iter_stack == iter.iter_stack);
        }
        
        bool operator !=(iterator iter)
        {
            return !(tree != iter.tree || iter_stack != iter.iter_stack);
        }

        Data& operator *(void)
        {
            return iter_stack.top()->data;
        }

        /// @return Label of node that iterator currently points at.
        const Label& label(void)
        {
            return iter_stack.top()->label;
        }
    };
    
    
    BinaryTree(Label l)
        :prev(NULL), next(NULL), label(l)
    {}
    
    iterator begin(void)
    {
        return iterator(this);
    }

    iterator end(void)
    {
        return iterator(this, true);
    }

    /// Get data under node with given label, creating it if needed.
    ///
    /// @todo Rewrite iteratively.
    Data& get_data(const Label l)
    {
        if (l == label)
            return data;
        else
        {
            BinaryTree<Label, Data> *&ptr = (l < label) ? next : prev;
            
            if (ptr == NULL)
                ptr = new BinaryTree(l);
            
            return ptr->get_data(l);
        }
    }
};

typedef BinaryTree<frequency_t, word_list > word_tree;

class Trie
{
private:
    /// Binary tree of words stored in root leaf of trie (sorted by
    /// frequency)
    word_tree words;

    /// Children tries
    vector<Trie*> children;
    
    /// Add word object under given full key
    void add_word_proc(const Word &w, const small_int &freq = 500, level_t level = 0)
    {
        if (w.str[level] == '\0')
            insert_word(words.get_data(freq), w);
        else
        {
            vector<Trie*>::size_type key = char_keys[w.str[level] - 'a'] - '1';
            if (children[key] == NULL)
                children[key] = new Trie();
            children[key]->add_word_proc(w, freq, level + 1);
        }
    }

    /// Get list of words stored in trie under given full key. We
    /// assume that all used words are present in the trie, so this
    /// always succeeds.
    word_tree& get_leaf(const char *full_key)
    {
        vector<Trie*>::size_type key = (full_key[0] - '1');
        if (*full_key == '\0')
            return words;
        else
        {
            return children[key]->get_leaf(full_key + 1);
        }
    }
public:
    Trie(void)
        :words(500)
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
        add_word_proc(Word(contents), freq);
    }

    /// Add new punctuation mark under 1
    void add_punctuation(const string &punct)
    {
        if (children[0] == NULL)
            children[0] = new Trie();
        insert_word(children[0]->words.get_data(500), Word(punct, false));
    }

    /// Get n-th word stored in trie under given full key.
    const Word& query(string &full_key, int n = 0)
    {
        word_tree &t = get_leaf(full_key.c_str());

        /// *t_iter is a list of words with same frequency
        word_tree::iterator t_iter = t.begin();
        word_list::iterator w_iter, w_enditer;

        do
        {
            /// Skip to next non-empty frequency if needed.
            while (!((*t_iter).size()))
                t_iter++;

            /// Begin iterating over words with this frequency
            w_iter = (*t_iter).begin();
            w_enditer = (*t_iter).end();

            /// Try to skip n items (or less)
            while ((w_iter != w_enditer) && n)
            {
                w_iter++;
                n--;
            }
                
            /// End of frequency reached?
            if (w_iter == w_enditer)
                t_iter++;
            else
                break;
        }
        while (1);

        /// Bump frequency and reinsert word if needed
        if (w_iter->bumpable)
        {
            Word w = Word(w_iter->str);
            /// Remove old word entry
            (*t_iter).erase(w_iter);
            
            w_iter = insert_word(t.get_data(t_iter.label() + 1), w, true);
        }

        return *w_iter;
    }
};

class T9Reader
{
private:
    string full_key;
    bool prev_punct, word_put;
    int skips;
    Trie *trie;

    /// If word selected so far has not been printed yet, do it
    void put_pending(void)
    {
        if (!word_put)
            put_current_word();
    }

public:
    T9Reader(Trie *t)
    {
        trie = t;
        full_key = "";
        word_put = true;
        prev_punct = false;
        skips = 0;
    }

    /// Print word selected so far
    void put_current_word(void)
    {
        trie->query(full_key, skips).print();
        full_key.clear();
        skips = 0;
        word_put = true;
        prev_punct = false;
    }
    
    void read(const string &input)
    {
        for (string::const_iterator i = input.begin(); i != input.end(); i++)
        {
            if (*i == ' ')
            {
                put_pending();
                cout << ' ';
            }
            else
            {
                /// Next character in word key
                if ((*i >= '2') && (*i <= '9'))
                {
                    if (prev_punct)
                        put_pending();
                    full_key += *i;
                    word_put = false;
                }
                else if (*i == '*')
                    skips++;
                /// Punctuation (put word)
                else if (*i == '1')
                {
                    put_pending();
                    full_key = "1";
                    word_put = false;
                    prev_punct = true;
                }
            }
        }
        put_pending();
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
