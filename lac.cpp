#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

/// Fit 40000 in 2 bytes
typedef unsigned short int small_int;

typedef vector <small_int> row;
typedef vector <row> matrix;
typedef vector <vector <unsigned long int> > lmatrix;

template <class M> void matrix_resize(M* m, small_int rows, small_int cols)
{
    m->resize(rows);
    for (typename M::iterator i = m->begin(); i != m->end(); i++)
        i->resize(cols, 0);
}

class Tree
{
private:
    /// Vertex count
    small_int size;

    /// Binary logarithm of size (rounded up)
    small_int levels;

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
    matrix anc;
    lmatrix dist;

    /// Traverse into v with p as parent
    void dfs_traverse(small_int v, small_int p = 0)
    {
        in_times[v] = in_timer++;
        
        anc[v][0] = p;
        dist[v][0] = m[p][v];

        for (small_int j = 1; j < levels; j++)
        {
            anc[v][j] = anc[anc[v][j - 1]][j - 1];
            dist[v][j] = dist[v][j - 1] + dist[anc[v][j - 1]][j - 1];
        }

        for (small_int i = 0; i < size; i++)
            if (m[v][i])
                dfs_traverse(i, v);

        out_times[v] = out_timer++;
    }
    friend void matrix_resize(matrix, small_int, small_int);

public:
    Tree(small_int n)
    {
        size = n;
        levels = ceil(log(size) / log(2));

        matrix_resize<matrix>(&m, n, n);
        matrix_resize<matrix>(&anc, n, levels);
        matrix_resize<lmatrix>(&dist, n, levels);
        
        in_times.resize(n, 0);
        out_times.resize(n, 0);
    }

    void print(void)
    {
        for (matrix::iterator i = anc.begin(); i != anc.end(); i++)
        {
            for (row::iterator e = i->begin(); e != i->end(); e++)
                cout << *e;
            cout << endl;
        }
        cout << "%%" << endl;
        
        for (lmatrix::iterator i = dist.begin(); i != dist.end(); i++)
        {
            for (vector<unsigned long int>::iterator e = i->begin(); e != i->end(); e++)
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
    tree->lac_preprocess();
    tree->print();
    
    cin >> pairs;
    
    for (small_int i = 0; i < pairs; i++)
    {
        cin >> a >> b;
        cout << tree->is_ancestor(a, b) << endl;
    }

    delete tree;
    return 0;
}
