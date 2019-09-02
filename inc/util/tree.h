#pragma once

#include <vector>

template <class T, typename Enable = void>
class TreeNode;

/**
 * Simple implementation of a Tree Node
 */
template <class T>
class TreeNodeBase {
protected:
    T *_node = nullptr; /**< pointer to the actual data this node holds */
    TreeNode<T> *_parent = nullptr; /**< ponter to parent Node. nullptr if root */
    std::vector<TreeNode<T> *> _children; /**< list of Pointers to child nodes */
public:
    /**
     * Constructor
     *
     * @param node
     *   The Object this node represents
     */
    TreeNodeBase<T>(T *node) : _node(node) {}

    /**
     * Getter for the _node member
     *
     * @returns
     *   The Object this node represents
     */
    T *node() {
        return this->_node;
    }

    /**
     * Getter for the _parent member
     *
     * @returns
     *   The Parent node in the tree
     */
    TreeNode<T> *parent() {
        return this->_parent;
    }

    /**
     * Getter for the _children member
     *
     * @returns
     *   A list of child-nodes in the tree
     */
    std::vector<TreeNode<T> *> children() {
        return this->_children;
    }

    /**
     * Find a node further down the tree representing a given Object
     *
     * @param node
     *   pointer to the Object to find
     * @returns
     *   Node representing the given object. nullptr if not found.
     */
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

    /**
     * construct a Node for the given objetc and set it as a child
     *
     * @param child
     *   object to construct child node for
     * @returns
     *   The added child node
     */
    TreeNode<T> *add_child(T *child) {
        TreeNodeBase<T> * t = new TreeNodeBase{child};
        this->_children.push_back(t);
        return t;
    }
};

/**
 * TreeNode class inheriting from TreeNodeBase
 *
 * This inheritance takes place for a simpler implementation of more specified trees
 */
template <class T, typename Enable>
class TreeNode : public TreeNodeBase<T> {};

/**
 * Simple implementation of a Tree
 */
template <class T>
class TreeBase {
protected:
    TreeNode<T> *root = nullptr; /**< pointer to the root node of the tree */
public:
    /**
     * construct a node for the given object and set it as root of the tree
     *
     * @param node
     *   Object to construct root node for
     * @returns
     *   The added root node
     */
    TreeNode<T> *add_root(T *node) {
        this->root =  new TreeNodeBase<T>(node);
        return this->root;
    }

    /**
     * construct a node for the given object and set it as child of a given parent
     *
     * @param node
     *   Object to construct node for
     * @param parent
     *   Object to add the constructed Node as child to
     * @returns
     *   constructed node
     */
    TreeNode<T> *add(T *node, T *parent) {
        TreeNodeBase<T> *parent_node = this->find(parent);
        if (parent_node == nullptr) {
            return nullptr;
        }
        parent_node->add_child(node);
    }

    /**
     * finds the given object in the tree
     *
     * @param node
     *   Object to find in tree
     * @returns
     *   Node representing the given object. nullptr if not in tree.
     */
    TreeNode<T> *find(T *node) const {
        if (this->root == nullptr) {
            return nullptr;
        }
        return this->root->find(node);
    }
};

/**
 * Tree class inheriting from TreeBase
 *
 * This inheritance takes place for a simpler implementation of more specified trees
 */
template <class T, typename Enable = void>
class Tree : public TreeBase<T> {};
