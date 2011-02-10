/// LCA algorithm.
/// 
/// Copyright (C) 2010, 2011 by Dmitry Dzhus
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions
/// are met:
///
///    1. Redistributions of source code must retain the above
///    copyright notice, this list of conditions and the following
///    disclaimer.
///
///    2. Redistributions in binary form must reproduce the above
///    copyright notice, this list of conditions and the following
///    disclaimer in the documentation and/or other materials provided
///    with the distribution.
///
///    3. The name of the author may not be used to endorse or promote
///    products derived from this software without specific prior
///    written permission.


#include <iostream>
#include <cmath>
#include <vector>
#include <list>

using namespace std;

/// Fit 40000 in 2 bytes
typedef unsigned short int small_int;

typedef vector <bool> brow;
typedef vector <small_int> row;

/// We use long integers here to fit maximum value of 40000×39999.
typedef unsigned long int distance_int;

template <class M> void matrix_resize(M* m, small_int rows, small_int cols)
{
    m->resize(rows);
    for (typename M::iterator i = m->begin(); i != m->end(); i++)
        i->resize(cols, 0);
}

/// Tree class
///
/// Implements LCA algorithm to calculate distance between two nodes
/// using Tree::find_distance() method.
class Tree
{
    typedef vector < vector <small_int> > anc_t;
    typedef vector < vector <distance_int> > anc_dist_t;

private:
    /// Vertex count
    small_int size;

    /// Binary logarithm of size (rounded up)
    small_int levels;

    /// Adjacency matrix. Our trees have sparse matrices, thus we use
    /// vectors of lists.
    vector< list <small_int> > adj;

    /// Distance matrix.
    vector< list <small_int> > dist;
    
    /// Node visit times after full DFS.
    vector<small_int> in_times, out_times;

    /// DFS timers
    small_int in_timer, out_timer;

    /// DFS visit markers
    vector<bool> visited;

    /// 2^j-th ancestors of each node.
    anc_t anc;

    /// Distances to 2^j-th ancestors of each node.
    anc_dist_t anc_dist;

    /// Traverse into v with p as parent
    void dfs_traverse(small_int v, small_int p = 0, small_int p_dist = 0)
    {
        visited[v] = 1;
        in_times[v] = in_timer++;
        
        anc[v][0] = p;
        anc_dist[v][0] = p_dist;

        for (small_int j = 1; j < levels; j++)
        {
            anc[v][j] = anc[anc[v][j - 1]][j - 1];
            anc_dist[v][j] = anc_dist[v][j - 1] + anc_dist[anc[v][j - 1]][j - 1];
        }

        /// Traverse adjacency and distance lists. This works provided
        /// that adjacency relations and edge lengths are added with
        /// add_edge which maintains proper order.
        for (list<small_int>::iterator 
                 i = adj[v].begin(), 
                 d = dist[v].begin(); 
             i != adj[v].end(); i++, d++)
            if (!visited[*i])
                dfs_traverse(*i, v, *d);

        out_times[v] = out_timer++;
    }

    small_int lca_proc(small_int v1, small_int v2)
    {
        for (int j = levels - 1; j >= 0; j--)
            if (!is_ancestor(anc[v1][j], v2))
                v1 = anc[v1][j];
        return anc[v1][0];
    }

    distance_int dist_to_ancestor(small_int v, small_int a)
    {
        distance_int r = 0;
        if (v == a)
            return 0;
        
        for (int j = levels - 1; j >= 0; j--)
            if (is_ancestor(a, anc[v][j]))
            {
                r += anc_dist[v][j];
                v = anc[v][j];
            }
        return r + anc_dist[v][0];
    }

public:
    Tree(small_int n)
    {
        size = n;
        levels = (n != 1) ? ceil(log(n) / log(2)) : 1;

        dist.resize(n);
        adj.resize(n);

        matrix_resize<anc_t>(&anc, n, levels);
        matrix_resize<anc_dist_t>(&anc_dist, n, levels);
        
        in_times.resize(n, 0);
        out_times.resize(n, 0);
        visited.resize(n, 0);
    }

    /// Add edge from v1 to v2 with given length
    void add_edge(small_int v1, small_int v2, small_int length)
    {
        dist[v1].push_back(length);
        dist[v2].push_back(length);
        adj[v1].push_back(v2);
        adj[v2].push_back(v1);
    }

    void lca_preprocess(void)
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

    /// Find LCA of two vertices
    small_int find_lca(small_int v1, small_int v2)
    {
        small_int v;
        if (is_ancestor(v1, v2))
            v = v1;
        else if (is_ancestor(v2, v1))
            v = v2;
        else 
            v = lca_proc(v1, v2);
        return v + 1;
    }

    /// Find distance between two vertices
    distance_int find_distance(small_int v1, small_int v2)
    {
        small_int lca = find_lca(v1, v2) - 1;
        /// @internal We can save one call if we calculate one of
        /// distance while finding LCA.
        return (v1 == v2) ? 0 : 
            dist_to_ancestor(v1, lca) +
            dist_to_ancestor(v2, lca);
    }
 
};

/// Read one integer N for node count. Then read N-1 integer 3-tuples
/// for edges: START END DISTANCE. Then read integer M and eventually
/// read M integer 2-tuples for pairs of nodes for which distance is
/// calculated and printed.
///
/// Example input:
/// 6
/// 1 2 7
/// 2 3 3
/// 2 4 6
/// 4 5 3
/// 5 6 1
/// 4
/// 1 6
/// 1 3
/// 6 3
/// 2 5
///
/// Output:
/// 17
/// 10
/// 13
/// 9

int main(int argc, char* argv[])
{
    small_int size, a, b, length, pairs;

    cin >> size;
    Tree tree(size);

    for (small_int i = 0; i < size - 1; i++)
    {
        cin >> a >> b >> length;
        tree.add_edge(a - 1, b - 1, length);
    }

    tree.lca_preprocess();
    cin >> pairs;
    
    for (small_int i = 0; i < pairs; i++)
    {
        cin >> a >> b;
        cout << tree.find_distance(a - 1, b - 1) << endl;
    }

    return 0;
}
