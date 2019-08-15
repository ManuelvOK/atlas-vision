#pragma once

#include <vector>

template <class T, typename Enable = void>
class TreeNode;

template <class T>
class TreeNodeBase {
protected:
    T *_node = nullptr;
    TreeNode<T> *_parent = nullptr;
    std::vector<TreeNode<T> *> _children;
public:
    TreeNodeBase<T>(T *node) : _node(node) {}

    T *node() {
        return this->_node;
    }

    TreeNode<T> *parent() {
        return this->_parent;
    }

    std::vector<TreeNode<T> *> children() {
        return this->_children;
    }

    TreeNode<T> *find(T *node) const {
        if (this->node == node) {
            return this;
        }
        for (TreeNodeBase<T> *n: this->children) {
            TreeNodeBase<T> *found = n->find(node);
            if (found != nullptr) {
                return found;
            }
        }
        return nullptr;
    }

    TreeNode<T> *add_child(T *child) {
        TreeNodeBase<T> * t = new TreeNodeBase{child};
        this->_children.push_back(t);
        return t;
    }
};

template <class T, typename Enable>
class TreeNode : public TreeNodeBase<T> {};

template <class T>
class TreeBase {
protected:
    TreeNode<T> *root = nullptr;
public:
    TreeNode<T> *add_root(T *node) {
        this->root =  new TreeNodeBase<T>(node);
        return this->root;
    }

    TreeNode<T> *add(T *node, T *parent) {
        TreeNodeBase<T> *parent_node = this->find(parent);
        if (parent_node == nullptr) {
            return nullptr;
        }
        parent_node->add_child(node);
    }

    TreeNode<T> *find(T *node) const {
        if (this->root == nullptr) {
            return nullptr;
        }
        return this->root->find(node);
    }
};

template <class T, typename Enable = void>
class Tree : public TreeBase<T> {};
