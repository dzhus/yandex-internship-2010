#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

/// Fit 40000 in 2 bytes
typedef unsigned short int small_int;

typedef vector <small_int> row;
typedef vector <row> matrix;

typedef unsigned long int distance_int;
typedef vector <vector <distance_int> > lmatrix;

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

    /// 2^j-th ancestors of each node.
    matrix anc;

    /// Distances to 2^j-th ancestors of each node. We use long
    /// integers here to fit maximum value of 40000×39999.
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

    small_int lac_proc(small_int v1, small_int v2)
    {
        for (int j = levels; j >= 0; j--)
            if (!is_ancestor(anc[v1][j], v2))
                v1 = anc[v1][j];
        return anc[v1][0];
    }

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
            for (vector<distance_int>::iterator e = i->begin(); e != i->end(); e++)
                cout << *e;
            cout << endl;
        }

    }

    /// Add edge from v1 to v2 with given length
    void add_edge(small_int v1, small_int v2, small_int length)
    {
        m[v1][v2] = length;
    }

    void lac_preprocess(void)
    {
        in_timer = out_timer = 0;
        dfs_traverse(0);
    }

    /// Return true if v1 is ancestor of v2 (1-based indexing)
    bool is_ancestor(small_int v1, small_int v2)
    {
        return (in_times[v1] < in_times[v2]) && 
            (out_times[v1] > out_times[v2]);
    }

    small_int find_lac(small_int v1, small_int v2)
    {
        small_int v;
        if (is_ancestor(v1, v2))
            v = v1;
        else if (is_ancestor(v2, v1))
            v = v2;
        else 
            v = lac_proc(v1, v2);
        return v + 1;
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
        tree->add_edge(a - 1, b - 1, length);
    }
    tree->lac_preprocess();
    
    cin >> pairs;
    
    for (small_int i = 0; i < pairs; i++)
    {
        cin >> a >> b;
        cout << tree->find_lac(a - 1, b - 1) << endl;
    }

    delete tree;
    return 0;
}
