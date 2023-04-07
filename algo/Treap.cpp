#include <bits/stdc++.h>
using namespace std;
 
#define ll long long
#define int long long
 
const ll MOD = 1000000007;
const int MP = 200010;
 
ll pw[MP], ip[MP];
 
// Modular exponentiation: computes b^e mod m.
ll mp(ll b, ll e, ll m = MOD) {
    ll r = 1;
    b %= m;
    while(e) {
        if(e & 1)
            r = (r * b) % m;
        b = (b * b) % m;
        e >>= 1;
    }
    return r;
}
 
// Precompute powers of 2 and their inverses modulo MOD.
void pre() {
    pw[0] = 1;
    for (int i = 1; i < MP; i++)
        pw[i] = (pw[i-1] * 2) % MOD;
    ip[0] = 1;
    ll inv2 = mp(2, MOD - 2);
    for (int i = 1; i < MP; i++)
        ip[i] = (ip[i-1] * inv2) % MOD;
}
 
// Disjoint Set Union (Union-Find) data structure.
struct DSU {
    vector<int> par, rk;
    DSU(int n) {
        par.resize(n+1);
        rk.assign(n+1, 0);
        for (int i = 0; i <= n; i++) 
            par[i] = i;
    }
    int find(int a) {
        return (par[a] == a ? a : par[a] = find(par[a]));
    }
    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if(a == b) return;
        if(rk[a] < rk[b])
            swap(a, b);
        par[b] = a;
        if(rk[a] == rk[b])
            rk[a]++;
    }
};
 
// Fenwick Tree (Binary Indexed Tree) for prefix sum queries.
struct Fenw {
    int n;
    vector<int> bit;
    Fenw(int n) : n(n) {
        bit.assign(n+1, 0);
    }
    void upd(int i, int delta) {
        for(; i <= n; i += i & -i)
            bit[i] += delta;
    }
    int query(int i) {
        int sum = 0;
        for(; i > 0; i -= i & -i)
            sum += bit[i];
        return sum;
    }
};
 
// Generic Treap Node structure.
struct N {
    int key, pri, sz;
    N *l, *r;
    N(int _key) : key(_key) {
        pri = rand();
        sz = 1;
        l = r = nullptr;
    }
};
 
// Get the size of subtree rooted at t.
int gs(N *t) {
    return t ? t->sz : 0;
}
 
// Update node t's size from its children.
void upd(N *t) {
    if(!t) return;
    t->sz = 1 + gs(t->l) + gs(t->r);
}
 
// Merge two treaps L and R.
N* mrgT(N *L, N *R) {
    if(!L || !R)
        return L ? L : R;
    if(L->pri > R->pri) {
        L->r = mrgT(L->r, R);
        upd(L);
        return L;
    } else {
        R->l = mrgT(L, R->l);
        upd(R);
        return R;
    }
}
 
// Split treap t into L and R such that keys in L are <= key.
void spl(N *t, int key, N* &L, N* &R) {
    if(!t) { L = R = nullptr; return; }
    if(t->key <= key) {
        spl(t->r, key, t->r, R);
        L = t;
        upd(L);
    } else {
        spl(t->l, key, L, t->l);
        R = t;
        upd(R);
    }
}
 
// Insert node it into treap t.
void ins(N* &t, N* it) {
    if(!t) { t = it; return; }
    if(it->pri > t->pri) {
        spl(t, it->key, it->l, it->r);
        t = it;
    } else if(it->key <= t->key) {
        ins(t->l, it);
    } else {
        ins(t->r, it);
    }
    upd(t);
}
 
// Erase the node with a given key from treap t.
void ers(N* &t, int key) {
    if(!t) return;
    if(t->key == key) {
        N* tmp = mrgT(t->l, t->r);
        delete t;
        t = tmp;
    } else if(key < t->key) {
        ers(t->l, key);
    } else {
        ers(t->r, key);
    }
    if(t)
        upd(t);
}
 
// Free the allocated memory in treap t.
void delT(N *t) {
    if(!t) return;
    delT(t->l);
    delT(t->r);
    delete t;
}
 
// Simple Linked List Node.
struct ListNode {
    int val;
    ListNode *next;
    ListNode(int _val) : val(_val), next(nullptr) {}
};
 
// Graph representation using an adjacency list.
struct Graph {
    int n;
    vector<vector<int>> adj;
    Graph(int n) : n(n) {
        adj.resize(n+1);
    }
    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u); // For undirected graphs.
    }
};
 
/*
   This template covers many commonly used data structures and algorithms,
   providing reusable utility functions for modular arithmetic, DSU, Fenwick trees,
   treaps, as well as basic linked list and graph representations.
   These components are widely applicable in various algorithmic challenges and
   systems programming tasks[3][4].
*/
 
signed main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    // Initialize modular arithmetic precomputations.
    pre();
    
    // Example usage for DSU.
    DSU dsu(10);
    dsu.unite(1, 2);
    dsu.unite(3, 4);
    
    // Example usage for Fenwick Tree.
    Fenw fenw(10);
    fenw.upd(3, 5);
    int sum = fenw.query(5);
    
    // Example usage for Treap.
    N* treap = nullptr;
    ins(treap, new N(10));
    ins(treap, new N(5));
    ins(treap, new N(15));
    ers(treap, 10);
    delT(treap);
    
    // Example usage for Linked List.
    ListNode* head = new ListNode(1);
    head->next = new ListNode(2);
    
    // Example usage for Graph.
    Graph graph(5);
    graph.addEdge(1, 2);
    graph.addEdge(1, 3);
    
    // Further extension and customization can be added as needed.
    
    return 0;
}
