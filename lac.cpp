#include <iostream>
#include <vector>

using namespace std;

/// Fit 40000 in 2 bytes
typedef unsigned short int small_int;

typedef vector <small_int> row;
typedef vector <row> matrix;

class Tree
{
private:
    small_int size;

    /// Distance matrix.
    /// 
    /// @internal If tree nodes are numbered from top to bottom, an
    /// upper triangular matrix can be used instead (saves nearly 50%
    /// of memory).
    matrix m;
    
    /// Node visit times after full DFS.
    row in_times, out_times;
    small_int in_timer, out_timer;

    /// 2^j-th ancestors of each node and distances to ancestors.
    matrix ancestors;
    vector <vector <unsigned long int> > distances;

    void dfs_traverse(small_int v)
    {
        in_times[v] = in_timer++;
        
        for (small_int i = 0; i < size; i++)
            if (m[v][i])
                dfs_traverse(i);

        out_times[v] = out_timer++;
    }

public:
    Tree(small_int n)
    {
        size = n;
        m.resize(n);
        for (matrix::iterator i = m.begin(); i != m.end(); i++)
            i->resize(n, 0);
        in_times.resize(n, 0);
        out_times.resize(n, 0);
    }

    void print(void)
    {
        for (matrix::iterator i = m.begin(); i != m.end(); i++)
        {
            for (row::iterator e = i->begin(); e != i->end(); e++)
                cout << *e;
            cout << endl;
        }
    }

    /// Add edge from v1 to v2 with given length (1-based indexing)
    void add_edge(small_int v1, small_int v2, small_int length)
    {
        m[v1 - 1][v2 - 1] = length;
    }

    void lac_preprocess(void)
    {
        in_timer = out_timer = 0;
        dfs_traverse(0);
    }

    /// Return true if v1 is ancestor of v2 (1-based indexing)
    bool is_ancestor(small_int v1, small_int v2)
    {
        return (in_times[v1 - 1] < in_times[v2 - 1]) && 
            (out_times[v1 - 1] > out_times[v2 - 1]);
    }
};

int main(int argc, char* argv[])
{
    small_int size, a, b, length, pairs;
    Tree *tree;

    cin >> size;
    tree = new Tree(size);

    for (small_int i = 0; i < size - 1; i++)
    {
        cin >> a >> b >> length;
        tree->add_edge(a, b, length);
    }
    tree->print();
    tree->lac_preprocess();
    
    cin >> pairs;
    
    for (small_int i = 0; i < pairs; i++)
    {
        cin >> a >> b;
        cout << tree->is_ancestor(a, b) << endl;
    }

    delete tree;
    return 0;
}
