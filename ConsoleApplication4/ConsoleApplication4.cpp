// ConsoleApplication4.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
// 此程式會建立 m-way 搜尋樹和 B-tree，並提供插入與刪除功能。

#include <iostream>
#include <vector>
#include <queue>

// 節點結構
struct Node {
    int key;
    std::vector<Node*> children;
    Node(int k) : key(k) {}
};

// m-way 搜尋樹
class MWayTree {
public:
    MWayTree(int m) : m(m), root(nullptr) {}

    void insert(int key) {
        if (!root) {
            root = new Node(key);
        }
        else {
            insertRec(root, key);
        }
    }

    void remove(int key) {
        if (root) {
            root = removeRec(root, key);
        }
    }

    void printTree() {
        if (root) {
            printRec(root, 0);
        }
    }

private:
    int m;
    Node* root;

    void insertRec(Node* node, int key) {
        if (node->children.size() < m) {
            node->children.push_back(new Node(key));
        }
        else {
            insertRec(node->children[0], key); // 簡單起見，插入到第一個子節點
        }
    }

    Node* removeRec(Node* node, int key) {
        if (!node) return nullptr;
        if (node->key == key) {
            if (node->children.empty()) {
                delete node;
                return nullptr;
            }
            else {
                node->key = node->children.back()->key;
                node->children.pop_back();
            }
        }
        else {
            for (auto& child : node->children) {
                child = removeRec(child, key);
            }
        }
        return node;
    }

    void printRec(Node* node, int level) {
        for (int i = 0; i < level; ++i) std::cout << "  ";
        std::cout << node->key << std::endl;
        for (Node* child : node->children) {
            printRec(child, level + 1);
        }
    }
};

// B-tree 節點結構
struct BTreeNode {
    int* keys;
    int t;
    BTreeNode** C;
    int n;
    bool leaf;

    BTreeNode(int _t, bool _leaf);

    void insertNonFull(int k);
    void splitChild(int i, BTreeNode* y);
    void traverse(int level);
    BTreeNode* search(int k);
    int findKey(int k);
    void remove(int k);
    void removeFromLeaf(int idx);
    void removeFromNonLeaf(int idx);
    int getPred(int idx);
    int getSucc(int idx);
    void fill(int idx);
    void borrowFromPrev(int idx);
    void borrowFromNext(int idx);
    void merge(int idx);

    friend class BTree;
};

BTreeNode::BTreeNode(int t1, bool leaf1) {
    t = t1;
    leaf = leaf1;
    keys = new int[2 * t - 1];
    C = new BTreeNode * [2 * t];
    n = 0;
}

void BTreeNode::traverse(int level) {
    int j = 0;
    for (int i = 0; i < level; ++i) std::cout << "  ";
    for (j = 0; j < n; j++) {
        if (leaf == false)
            C[j]->traverse(level + 1);
        std::cout << keys[j] << " ";
    }
    if (leaf == false)
        C[j]->traverse(level + 1);
    std::cout << std::endl;
}

BTreeNode* BTreeNode::search(int k) {
    int i = 0;
    while (i < n && k > keys[i])
        i++;
    if (keys[i] == k)
        return this;
    if (leaf == true)
        return nullptr;
    return C[i]->search(k);
}

void BTreeNode::insertNonFull(int k) {
    int i = n - 1;
    if (leaf == true) {
        while (i >= 0 && keys[i] > k) {
            keys[i + 1] = keys[i];
            i--;
        }
        keys[i + 1] = k;
        n = n + 1;
    }
    else {
        while (i >= 0 && keys[i] > k)
            i--;
        if (C[i + 1]->n == 2 * t - 1) {
            splitChild(i + 1, C[i + 1]);
            if (keys[i + 1] < k)
                i++;
        }
        C[i + 1]->insertNonFull(k);
    }
}

void BTreeNode::splitChild(int i, BTreeNode* y) {
    BTreeNode* z = new BTreeNode(y->t, y->leaf);
    z->n = t - 1;
    for (int j = 0; j < t - 1; j++)
        z->keys[j] = y->keys[j + t];
    if (y->leaf == false) {
        for (int j = 0; j < t; j++)
            z->C[j] = y->C[j + t];
    }
    y->n = t - 1;
    for (int j = n; j >= i + 1; j--)
        C[j + 1] = C[j];
    C[i + 1] = z;
    for (int j = n - 1; j >= i; j--)
        keys[j + 1] = keys[j];
    keys[i] = y->keys[t - 1];
    n = n + 1;
}

void BTreeNode::remove(int k) {
    int idx = findKey(k);
    if (idx < n && keys[idx] == k) {
        if (leaf)
            removeFromLeaf(idx);
        else
            removeFromNonLeaf(idx);
    }
    else {
        if (leaf) {
            std::cout << "The key " << k << " is does not exist in the tree\n";
            return;
        }
        bool flag = ((idx == n) ? true : false);
        if (C[idx]->n < t)
            fill(idx);
        if (flag && idx > n)
            C[idx - 1]->remove(k);
        else
            C[idx]->remove(k);
    }
    return;
}

int BTreeNode::findKey(int k) {
    int idx = 0;
    while (idx < n && keys[idx] < k)
        ++idx;
    return idx;
}

void BTreeNode::removeFromLeaf(int idx) {
    for (int i = idx + 1; i < n; ++i)
        keys[i - 1] = keys[i];
    n--;
    return;
}

void BTreeNode::removeFromNonLeaf(int idx) {
    int k = keys[idx];
    if (C[idx]->n >= t) {
        int pred = getPred(idx);
        keys[idx] = pred;
        C[idx]->remove(pred);
    }
    else if (C[idx + 1]->n >= t) {
        int succ = getSucc(idx);
        keys[idx] = succ;
        C[idx + 1]->remove(succ);
    }
    else {
        merge(idx);
        C[idx]->remove(k);
    }
    return;
}

int BTreeNode::getPred(int idx) {
    BTreeNode* cur = C[idx];
    while (!cur->leaf)
        cur = cur->C[cur->n];
    return cur->keys[cur->n - 1];
}

int BTreeNode::getSucc(int idx) {
    BTreeNode* cur = C[idx + 1];
    while (!cur->leaf)
        cur = cur->C[0];
    return cur->keys[0];
}

void BTreeNode::fill(int idx) {
    if (idx != 0 && C[idx - 1]->n >= t)
        borrowFromPrev(idx);
    else if (idx != n && C[idx + 1]->n >= t)
        borrowFromNext(idx);
    else {
        if (idx != n)
            merge(idx);
        else
            merge(idx - 1);
    }
    return;
}

void BTreeNode::borrowFromPrev(int idx) {
    BTreeNode* child = C[idx];
    BTreeNode* sibling = C[idx - 1];
    for (int i = child->n - 1; i >= 0; --i)
        child->keys[i + 1] = child->keys[i];
    if (!child->leaf) {
        for (int i = child->n; i >= 0; --i)
            child->C[i + 1] = child->C[i];
    }
    child->keys[0] = keys[idx - 1];
    if (!child->leaf)
        child->C[0] = sibling->C[sibling->n];
    keys[idx - 1] = sibling->keys[sibling->n - 1];
    child->n += 1;
    sibling->n -= 1;
    return;
}

void BTreeNode::borrowFromNext(int idx) {
    BTreeNode* child = C[idx];
    BTreeNode* sibling = C[idx + 1];
    child->keys[(child->n)] = keys[idx];
    if (!(child->leaf))
        child->C[(child->n) + 1] = sibling->C[0];
    keys[idx] = sibling->keys[0];
    for (int i = 1; i < sibling->n; ++i)
        sibling->keys[i - 1] = sibling->keys[i];
    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->n; ++i)
            sibling->C[i - 1] = sibling->C[i];
    }
    child->n += 1;
    sibling->n -= 1;
    return;
}
void BTreeNode::merge(int idx) {
    BTreeNode* child = C[idx];
    BTreeNode* sibling = C[idx + 1];
    child->keys[t - 1] = keys[idx];
    for (int i = 0; i < sibling->n; ++i)
        child->keys[i + t] = sibling->keys[i];
    if (!child->leaf) {
        for (int i = 0; i <= sibling->n; ++i)
            child->C[i + t] = sibling->C[i];
    }
    for (int i = idx + 1; i < n; ++i)
        keys[i - 1] = keys[i];
    for (int i = idx + 2; i <= n; ++i)
        C[i - 1] = C[i];
    child->n += sibling->n + 1;
    n--;
    delete sibling;
    return;
}

// B-tree
class BTree {
public:
    BTree(int _t) : t(_t), root(nullptr) {}

    void insert(int k) {
        if (root == nullptr) {
            root = new BTreeNode(t, true);
            root->keys[0] = k;
            root->n = 1;
        }
        else {
            if (root->n == 2 * t - 1) {
                BTreeNode* s = new BTreeNode(t, false);
                s->C[0] = root;
                s->splitChild(0, root);
                int i = 0;
                if (s->keys[0] < k)
                    i++;
                s->C[i]->insertNonFull(k);
                root = s;
            }
            else
                root->insertNonFull(k);
        }
    }

    void remove(int k) {
        if (!root) {
            std::cout << "The tree is empty\n";
            return;
        }
        root->remove(k);
        if (root->n == 0) {
            BTreeNode* tmp = root;
            if (root->leaf)
                root = nullptr;
            else
                root = root->C[0];
            delete tmp;
        }
        return;
    }

    void traverse() {
        if (root != nullptr) traverseRec(root, 0);
    }

private:
    BTreeNode* root;
    int t;

    void traverseRec(BTreeNode* node, int level) {
        for (int i = 0; i < level; ++i) std::cout << "  ";
        for (int i = 0; i < node->n; ++i) {
            std::cout << node->keys[i] << " ";
        }
        std::cout << std::endl;
        if (!node->leaf) {
            for (int i = 0; i <= node->n; ++i) {
                traverseRec(node->C[i], level + 1);
            }
        }
    }
};

int main() {
    int m, t;
    std::cout << "請輸入 m-way 搜尋樹的 m 值: ";
    std::cin >> m;
    std::cout << "請輸入 B-tree 的 t 值: ";
    std::cin >> t;

    MWayTree mWayTree(m);
    BTree bTree(t);

    std::vector<int> values;
    int value;
    std::cout << "請輸入整數值 (輸入 -1 結束): ";
    while (std::cin >> value && value != -1) {
        values.push_back(value);
    }

    for (int v : values) {
        mWayTree.insert(v);
        bTree.insert(v);
    }

    std::cout << "m-way 搜尋樹: " << std::endl;
    mWayTree.printTree();

    std::cout << "B-tree: " << std::endl;
    bTree.traverse();

    std::cout << "請輸入要插入的值: ";
    std::cin >> value;
    mWayTree.insert(value);
    bTree.insert(value);

    std::cout << "請輸入要刪除的值: ";
    std::cin >> value;
    mWayTree.remove(value);
    bTree.remove(value);

    std::cout << "更新後的 m-way 搜尋樹: " << std::endl;
    mWayTree.printTree();

    std::cout << "更新後的 B-tree: " << std::endl;
    bTree.traverse();

    return 0;
}