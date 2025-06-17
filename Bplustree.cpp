#include <bits/stdc++.h>
using namespace std;

class Node {
public:
    bool leaf;
    vector<int> keys;
    vector<Node*> children;
    Node *parent;
    Node *next;
    Node *prev;

    Node(bool is_leaf = false) {
        leaf = is_leaf;
        parent = nullptr;
        next = nullptr;
        prev = nullptr;
    }
};

class Bplustree {
private:
    int degree;
    Node *root;

public:
    Bplustree(int m = 4) {
        degree = m;
        root = new Node;
        root->leaf = true;
    }

    // Insert a value into the B+ tree
    void insert(int val) {
        Node *curr = root;

        // Traverse to the appropriate leaf node
        while (!curr->leaf) {
            int i = 0;
            while (i < curr->keys.size() && val >= curr->keys[i]) {
                i++;
            }
            curr = curr->children[i];
        }

        // Insert the value into the leaf node
        curr->keys.insert(lower_bound(curr->keys.begin(), curr->keys.end(), val), val);

        // Split the node if overflow occurs
        if (curr->keys.size() == degree) {
            split(curr);
        }
    }

    // Split a leaf or internal node
    void split(Node *node) {
        int mid = degree / 2;
        Node *new_node = new Node(node->leaf); // Create a new node

        // Assign half the keys to the new node
        new_node->keys.assign(node->keys.begin() + mid, node->keys.end());
        node->keys.erase(node->keys.begin() + mid, node->keys.end());

        // Handle leaf node case
        if (node->leaf) {
            new_node->next = node->next;
            if (node->next != nullptr) {
                node->next->prev = new_node;
            }
            node->next = new_node;
            new_node->prev = node;
        }

        if (node->parent == nullptr) {
            // Create a new root if splitting the root
            root = new Node(false);
            root->keys.push_back(new_node->keys[0]);
            root->children.push_back(node);
            root->children.push_back(new_node);
            node->parent = root;
            new_node->parent = root;
        } else {
            // Insert into the parent node
            Node *parent = node->parent;
            parent->keys.insert(lower_bound(parent->keys.begin(), parent->keys.end(), new_node->keys[0]), new_node->keys[0]);
            parent->children.insert(parent->children.begin() + (find(parent->children.begin(), parent->children.end(), node) - parent->children.begin()) + 1, new_node);
            new_node->parent = parent;

            // Split the parent if overflow occurs
            if (parent->keys.size() == degree) {
                split_parent(parent);
            }
        }
    }

    // Split an internal node
    void split_parent(Node *node) {
        int mid = degree / 2;
        Node *new_node = new Node(false);
        int mid_val = node->keys[mid];

        // Assign half the keys and children to the new node
        new_node->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
        node->keys.erase(node->keys.begin() + mid, node->keys.end());
        new_node->children.assign(node->children.begin() + mid + 1, node->children.end());
        node->children.erase(node->children.begin() + mid + 1, node->children.end());

        // Update children's parent pointers
        for (Node *child : new_node->children) {
            child->parent = new_node;
        }

        if (node->parent == nullptr) {
            // Create a new root if splitting the root
            root = new Node(false);
            root->keys.push_back(mid_val);
            root->children.push_back(node);
            root->children.push_back(new_node);
            node->parent = root;
            new_node->parent = root;
        } else {
            // Insert into the parent node
            Node *parent = node->parent;
            parent->keys.insert(lower_bound(parent->keys.begin(), parent->keys.end(), mid_val), mid_val);
            parent->children.insert(parent->children.begin() + (find(parent->children.begin(), parent->children.end(), node) - parent->children.begin()) + 1, new_node);
            new_node->parent = parent;

            // Split the parent if overflow occurs
            if (parent->keys.size() == degree) {
                split_parent(parent);
            }
        }
    }

    // Remove a value from the B+ tree
    void remove(int val) {
        Node *curr = root;

        // Traverse to the appropriate leaf node
        while (!curr->leaf) {
            int i = 0;
            while (i < curr->keys.size() && val >= curr->keys[i]) {
                i++;
            }
            curr = curr->children[i];
        }

        // Remove the value from the leaf node
        auto it = find(curr->keys.begin(), curr->keys.end(), val);
        if (it == curr->keys.end()) {
            cout << "Value does not exist!" << endl;
            return;
        }
        curr->keys.erase(it);

        // Handle underflow if it occurs
        if (curr->keys.size() < ceil((degree - 1) / 2)) {
            node_underflow(curr);
            return;
        }

        // Update guide keys in the parent nodes
        int guide = curr->keys[0];
        while (curr->parent != nullptr) {
            Node *parent = curr->parent;
            it = find(parent->keys.begin(), parent->keys.end(), val);
            if (it != parent->keys.end()) {
                parent->keys.erase(it);
                parent->keys.insert(lower_bound(parent->keys.begin(), parent->keys.end(), guide), guide);
            }
            curr = curr->parent;
        }
    }

    // Handle underflow in a node
    void node_underflow(Node *node) {
        if (node->parent == nullptr) {
            if (node->keys.empty()) {
                if (!node->leaf) {
                    root = node->children[0];
                    root->parent = nullptr;
                } else {
                    root = nullptr;
                }
                delete node;
            }
            return;
        }

        Node *parent = node->parent;
        bool left = false, right = false;

        // Check for eligible siblings
        if (node->prev != nullptr && node->prev->parent == parent) {
            left = (node->prev->keys.size() - 1 > (degree / 2) - 1);
        }
        if (node->next != nullptr && node->next->parent == parent) {
            right = (node->next->keys.size() - 1 > (degree / 2) - 1);
        }

        // Borrow or merge based on sibling availability
        if (left) {
            Node *sibling = node->prev;
            int val = sibling->keys.back();
            node->keys.insert(lower_bound(node->keys.begin(), node->keys.end(), val), val);
            sibling->keys.pop_back();
            int i = find(parent->children.begin(), parent->children.end(), node) - parent->children.begin();
            parent->keys[i - 1] = val;
            return;
        }
        if (right) {
            Node *sibling = node->next;
            int val = sibling->keys.front();
            node->keys.insert(lower_bound(node->keys.begin(), node->keys.end(), val), val);
            sibling->keys.erase(sibling->keys.begin());
            int i = find(parent->children.begin(), parent->children.end(), node) - parent->children.begin();
            parent->keys[i] = sibling->keys[0];
            return;
        }
        if (!left && !right) {
            merge(node);
        }
    }

    // Merge two nodes
    void merge(Node *node) {
        Node *parent = node->parent;
        Node *sibling = nullptr;
        bool left_merge = false;

        // Determine whether to merge with the left or right sibling
        if (node->prev != nullptr && node->prev->parent == parent) {
            sibling = node->prev;
            left_merge = true;
        } else if (node->next != nullptr && node->next->parent == parent) {
            sibling = node->next;
        }

        if (left_merge) {
            sibling->keys.insert(sibling->keys.end(), node->keys.begin(), node->keys.end());
            sibling->next = node->next;
            if (node->next) {
                node->next->prev = sibling;
            }
            int idx = find(parent->children.begin(), parent->children.end(), node) - parent->children.begin();
            parent->keys.erase(parent->keys.begin() + (idx - 1));
            parent->children.erase(parent->children.begin() + idx);
            delete node;
        } else {
            sibling->keys.insert(sibling->keys.begin(), node->keys.begin(), node->keys.end());
            sibling->prev = node->prev;
            if (node->prev) {
                node->prev->next = sibling;
            }
            int idx = find(parent->children.begin(), parent->children.end(), node) - parent->children.begin();
            parent->keys.erase(parent->keys.begin() + idx);
            parent->children.erase(parent->children.begin() + idx);
            delete node;
        }

        if (parent->keys.size() < ceil(degree / 2)) {
            internal_underflow(parent);
        }
    }

    // Handle underflow in an internal node
    void internal_underflow(Node *node) {
        if (node == root) {
            if (node->keys.empty() && !node->leaf) {
                root = node->children[0];
                root->parent = nullptr;
                delete node;
            }
            return;
        }

        Node *parent = node->parent;
        int idx = find(parent->children.begin(), parent->children.end(), node) - parent->children.begin();
        Node *left_sibling = (idx > 0) ? parent->children[idx - 1] : nullptr;
        Node *right_sibling = (idx < parent->children.size() - 1) ? parent->children[idx + 1] : nullptr;

        if (left_sibling && left_sibling->keys.size() > (degree / 2)) {
            node->keys.insert(node->keys.begin(), parent->keys[idx - 1]);
            parent->keys[idx - 1] = left_sibling->keys.back();
            left_sibling->keys.pop_back();
            if (!left_sibling->leaf) {
                node->children.insert(node->children.begin(), left_sibling->children.back());
                left_sibling->children.pop_back();
                node->children[0]->parent = node;
            }
        } else if (right_sibling && right_sibling->keys.size() > (degree / 2)) {
            node->keys.push_back(parent->keys[idx]);
            parent->keys[idx] = right_sibling->keys.front();
            right_sibling->keys.erase(right_sibling->keys.begin());
            if (!right_sibling->leaf) {
                node->children.push_back(right_sibling->children.front());
                right_sibling->children.erase(right_sibling->children.begin());
                node->children.back()->parent = node;
            }
        } else {
            if (left_sibling) {
                left_sibling->keys.push_back(parent->keys[idx - 1]);
                left_sibling->keys.insert(left_sibling->keys.end(), node->keys.begin(), node->keys.end());
                if (!node->leaf) {
                    left_sibling->children.insert(left_sibling->children.end(), node->children.begin(), node->children.end());
                    for (auto *child : node->children) {
                        child->parent = left_sibling;
                    }
                }
                parent->keys.erase(parent->keys.begin() + (idx - 1));
                parent->children.erase(parent->children.begin() + idx);
                delete node;
            } else if (right_sibling) {
                node->keys.push_back(parent->keys[idx]);
                node->keys.insert(node->keys.end(), right_sibling->keys.begin(), right_sibling->keys.end());
                if (!right_sibling->leaf) {
                    node->children.insert(node->children.end(), right_sibling->children.begin(), right_sibling->children.end());
                    for (auto *child : right_sibling->children) {
                        child->parent = node;
                    }
                }
                parent->keys.erase(parent->keys.begin() + idx);
                parent->children.erase(parent->children.begin() + (idx + 1));
                delete right_sibling;
            }
        }

        if (parent->keys.size() < (degree / 2)) {
            internal_underflow(parent);
        }
    }

    // Display the B+ tree in order
    void display() {
        Node *curr = root;
        if (curr == nullptr) {
            return;
        }
        while (!curr->leaf) {
            curr = curr->children[0];
        }
        while (curr != nullptr) {
            for (int i = 0; i < curr->keys.size(); i++) {
                cout << curr->keys[i] << " ";
            }
            curr = curr->next;
        }
    }

    // Search for a value in the B+ tree
    int search(int val) {
        Node *curr = root;
        while (!curr->leaf) {
            int i = 0;
            while (i < curr->keys.size() && val > curr->keys[i]) {
                i++;
            }
            curr = curr->children[i];
        }
        if (find(curr->keys.begin(), curr->keys.end(), val) != curr->keys.end()) {
            return 1;
        }
        return 0;
    }

    // Debugging function to display the structure of the tree
    void debug() {
        print(root, 0);
    }

    // Helper function to recursively print the tree
    void print(Node *node, int level) {
        if (node == nullptr) {
            return;
        }
        cout << "Level " << level << ": ";
        for (int i = 0; i < node->keys.size(); i++) {
            cout << node->keys[i] << " ";
        }
        cout << endl;
        if (!node->leaf) {
            for (Node *child : node->children) {
                print(child, level + 1);
            }
        }
    }
};

int main() {
    Bplustree tree(3);

    // Step 1: Insert initial values
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);
    tree.insert(40);
    tree.insert(50);
    cout << "Inserted 10, 20, 30, 40, 50\n";

    // Step 2: Insert more values
    tree.insert(60);
    tree.insert(70);
    tree.insert(80);
    tree.insert(90);
    cout << "Inserted 60, 70, 80, 90\n";

    // Step 3: Additional insertions
    tree.insert(25);
    tree.insert(35);
    tree.insert(45);
    tree.insert(55);
    tree.insert(65);
    cout << "Inserted 25, 35, 45, 55, 65\n";

    // Step 4: Remove a value
    tree.remove(50);
    cout << "Removed 50\n";

    // Step 5: Insert more values
    tree.insert(85);
    tree.insert(95);
    tree.insert(100);
    cout << "Inserted 85, 95, 100\n";

    // Further removals and insertions
    tree.remove(30);
    tree.remove(10);
    tree.remove(80);
    tree.insert(22);
    tree.insert(41);
    tree.insert(54);
    tree.remove(54);
    tree.remove(45);

    // Search for a value
    int found = tree.search(22);
    cout << (found ? "Found 22" : "22 not found") << endl;

    // Display the tree structure
    tree.debug();

    // Display the tree contents
    tree.display();

    return 0;
}
