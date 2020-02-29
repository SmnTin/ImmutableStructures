#ifndef IMMU_VECTOR_IMPL_H
#define IMMU_VECTOR_IMPL_H

#include <cstdint>
#include "immu/details/exceptions.h"
#include <iostream>

namespace immu::details {
    template<typename T, size_t CHUNK_SIZE>
    struct vector_node_t {
        typedef uint64_t size_t;
        enum class type_t {
            node,
            leaf
        } type = type_t::node;

        size_t size = 0;
        size_t max_capacity = CHUNK_SIZE;

        struct node_t {
            vector_node_t **nodes_ref = nullptr;

            static vector_node_t *empty_node() {
                auto *node = new vector_node_t();
                node->type = type_t::node;
                node->size = 0;
                node->node_impl.nodes_ref = new vector_node_t *[CHUNK_SIZE];
                std::fill(node->node_impl.nodes_ref,
                          node->node_impl.nodes_ref + CHUNK_SIZE,
                          nullptr);
                return node;
            }
        } node_impl;

        struct leaf_t {
            T **values_ref = nullptr;

            static vector_node_t *empty_leaf() {
                auto *node = new vector_node_t();
                node->type = type_t::leaf;
                node->size = 0;
                node->leaf_impl.values_ref = new T *[CHUNK_SIZE];
                std::fill(node->leaf_impl.values_ref,
                          node->leaf_impl.values_ref + CHUNK_SIZE,
                          nullptr);
                return node;
            }
        } leaf_impl;

        vector_node_t *clone() const {
            vector_node_t *node;
            if (type == type_t::node) {
                node = node_t::empty_node();
                std::copy(node_impl.nodes_ref,
                          node_impl.nodes_ref + CHUNK_SIZE,
                          node->node_impl.nodes_ref);
            } else {
                node = leaf_t::empty_leaf();
                std::copy(leaf_impl.values_ref,
                          leaf_impl.values_ref + CHUNK_SIZE,
                          node->leaf_impl.values_ref);
            }
            node->size = size;
            node->max_capacity = max_capacity;
            return node;
        }
    };

    template<typename T, size_t CHUNK_SIZE = 4U>
    class vector_impl {
    public:
        typedef uint64_t size_t;
        using node_t = vector_node_t<T, CHUNK_SIZE>;

        static vector_impl &&empty() {
            return std::move(vector_impl());
        }

        static vector_impl &&fill_with_value(size_t n, T &&value) {
            vector_impl vec;
            for (size_t i = 0; i < n; ++i)
                vec.push_back_move(value);
            return std::move(vec);
        }

        const T &at(size_t index) const {
            if (index >= size())
                throw random_access_out_of_range(index, size());
            return nonrecursive_get(index);
        }

        vector_impl &&push_back_copy(T &value) const {
            vector_impl vec;
            node_t *new_root = node_push_back(value, true);
            vec.root = new_root;
            vec.size_ = size_ + 1;
            return std::move(vec);
        }

        vector_impl &&push_back_move(T &value) {
            node_t *new_root = node_push_back(value, false);
            ++size_;
            root = new_root;
            return std::move(*this);
        }

        [[nodiscard]] size_t size() const {
            return size_;
        }

    protected:
        size_t size_ = 0;
        node_t *root = nullptr;

        const T &nonrecursive_get(size_t index) const {
            //implicit key tree traverse
            node_t *cur = root;
            while (cur->type != node_t::type_t::leaf) {
                size_t i;
                for (i = 0; index >= cur->node_impl.nodes_ref[i]->size; ++i)
                    index -= cur->node_impl.nodes_ref[i]->size;
                cur = cur->node_impl.nodes_ref[i];
            }
            return *cur->leaf_impl.values_ref[index];
        }

        static size_t node_size(node_t *node) {
            return node ? node->size : 0U;
        }

        node_t *node_push_back(T &value, bool copying) const {
            node_t *new_root = root;
            node_t *cur = root;
            size_t next_index = size_;

            if (!root) {
                new_root = node_t::leaf_t::empty_leaf();
            } else if (next_index >= cur->max_capacity) {
                new_root = node_t::node_t::empty_node();
                new_root->max_capacity = cur->max_capacity * CHUNK_SIZE;
                new_root->node_impl.nodes_ref[0] = root;
            } else if (copying && root) {
                new_root = root->clone();
            }
            cur = new_root;

            while (cur->type != node_t::type_t::leaf) {
                ++cur->size;

                size_t next_capacity = cur->max_capacity / CHUNK_SIZE;
                size_t i = next_index / next_capacity;
                next_index -= i * next_capacity;
                node_t *&next = cur->node_impl.nodes_ref[i];
                if (!next) {
                    if (next_capacity == CHUNK_SIZE)
                        next = node_t::leaf_t::empty_leaf();
                    else
                        next = node_t::node_t::empty_node();
                    next->max_capacity = next_capacity;
                } else if (copying) {
                    next = next->clone();
                }
                cur = next;
            }
            ++cur->size;
            cur->leaf_impl.values_ref[next_index] = new T(value);

            return new_root;
        }
    };
} //immu::details

#endif //IMMU_VECTOR_IMPL_H
