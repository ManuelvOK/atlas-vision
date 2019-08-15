#pragma once

#include<string>
#include<vector>

#include <util/attributable.h>
#include <util/tree.h>


template<class T>
class TreeNode<T, typename std::enable_if<std::is_base_of<Attributable, T>::value>::type> : TreeNodeBase<T> {
public:
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

template<class T>
class Tree<T, typename std::enable_if<std::is_base_of<Attributable, T>::value>::type> : TreeBase<T> {
public:
    T *find_first_with_attribute(std::string attribute) const {
        if (this->root == nullptr) {
            return nullptr;
        }
        return this->root->find_first_with_attribute(attribute);
    }

    std::vector<T *> find_all_with_attribute(std::string attribute) const {
        if (this->root == nullptr) {
            return std::vector<T *>();
        }
        return this->root->find_all_with_attribute(attribute);
    }
};
