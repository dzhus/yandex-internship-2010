#include <iostream>
#include <vector>

using namespace std;

/// Fit 40000 in 2 bytes
typedef unsigned short int small_int;

typedef vector<small_int> row;
typedef vector<row> matrix;

class Tree
{
public:
    Tree(small_int n)
    {
        m.resize(n);
        for (matrix::iterator i = m.begin(); i != m.end(); i++)
            i->resize(n);
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

    /// Add edge from a to b with given length (1-based indexing)
    void add_edge(small_int a, small_int b, small_int length)
    {
        m[a - 1][b - 1] = length;
    }
        
private:
    /// Distance matrix.
    /// 
    /// @internal If tree nodes are numbered from top to bottom, an
    /// upper triangular matrix can be used instead (saves nearly 50%
    /// of memory).
    matrix m;
};

int main(int argc, char* argv[])
{
    small_int size, a, b, length;
    Tree *tree;

    cin >> size;
    tree = new Tree(size);

    for (small_int i = 1; i < size; i++)
    {
        cin >> a >> b >> length;
        tree->add_edge(a, b, length);
    }
    tree->print();
    delete tree;
    return 0;
}
