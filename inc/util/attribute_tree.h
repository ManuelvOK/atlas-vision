#pragma once

#include<string>
#include<vector>

#include <util/attributable.h>
#include <util/tree.h>


/**
 * Node of an AttributeTree
 *
 * This holds the recursive algorithms to find Nodes based on Attributes
 */
template<class T>
class TreeNode<T, typename std::enable_if<std::is_base_of<Attributable, T>::value>::type> : TreeNodeBase<T> {
public:
    /**
     * Find first Child that holds a given attribute
     *
     * @param attribute
     *   attribute to search for
     * @returns
     *   first found Object that holds the given attribute. nullptr if no such object exists.
     */
    T *find_first_with_attribute(std::string attribute) const {
        if (this->_node->has_attribute(attribute)) {
            return this->_node;
        }
        for(TreeNode<T> *n: this->_children) {
            T *found = n->find_first_with_attribute(attribute);
            if (found != nullptr) {
                return found;
            }
        }
        return nullptr;
    }

    /**
     * Find all Childs that hold a given attribute
     *
     * @param attribute
     *   attribute to search for
     * @returns
     *   List of Objects that hold the given attribute.
     */
    std::vector<T *> find_all_with_attribute(std::string attribute) const {
        std::vector<T *> vec;
        if (this->_node->has_attribute(attribute)) {
            vec.push_back(this->_node);
        }
        for(TreeNode<T> *n: this->_children) {
            std::vector<T *> found = n->find_all_with_attribute(attribute);
            vec.insert(vec.end(), found.begin(), found.end());
        }
        return vec;
    }
};

/**
 * A Tree that holds Attributable Objects.
 *
 * It is possible to find one or more objects in this tree based on the given attributes
 */
template<class T>
class Tree<T, typename std::enable_if<std::is_base_of<Attributable, T>::value>::type> : TreeBase<T> {
public:
    /**
     * Find first Object in tree that holds a given attribute
     *
     * @param attribute
     *   Attribute to search for
     * @returns
     *   First found Object that holds the given attribute. nullptr if no such object exists.
     */
    T *find_first_with_attribute(std::string attribute) const {
        if (this->root == nullptr) {
            return nullptr;
        }
        return this->root->find_first_with_attribute(attribute);
    }


    /**
     * Find all Objects in tree that hold a given attribute
     *
     * @param attribute
     *   attribute to search for
     * @returns
     *   List of Objects that hold the given attribute.
     */
    std::vector<T *> find_all_with_attribute(std::string attribute) const {
        if (this->root == nullptr) {
            return std::vector<T *>();
        }
        return this->root->find_all_with_attribute(attribute);
    }
};
