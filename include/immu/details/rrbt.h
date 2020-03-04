#ifndef IMMU_RRBT_H
#define IMMU_RRBT_H

#include <cstdint>
#include <tuple>
#include "immu/details/exceptions.h"
#include <iostream>

namespace immu::details {
    namespace rrbt {

        template<typename T, size_t CHUNK_SIZE_BITS>
        struct rrbt_node_t {
            enum class type_t {
                node,
                leaf
            } type = type_t::node;

            size_t size = 0;
            static const size_t CHUNK_SIZE = (1u << CHUNK_SIZE_BITS);

            struct node_t {
                rrbt_node_t **nodes_ref = nullptr;
                size_t fill = 0;

                static rrbt_node_t *empty_node() {
                    auto *node = new rrbt_node_t();
                    node->type = type_t::node;
                    node->size = 0;
                    node->node_impl.nodes_ref = new rrbt_node_t *[CHUNK_SIZE];
                    std::fill(node->node_impl.nodes_ref,
                              node->node_impl.nodes_ref + CHUNK_SIZE,
                              nullptr);
                    return node;
                }

                rrbt_node_t *&last() {
                    return nodes_ref[fill - 1];
                }

                const rrbt_node_t *last() const {
                    return nodes_ref[fill - 1];
                }
            } node_impl;

            struct leaf_t {
                T **values_ref = nullptr;
                size_t fill = 0;

                static rrbt_node_t *empty_leaf() {
                    auto *node = new rrbt_node_t();
                    node->type = type_t::leaf;
                    node->size = 0;
                    node->leaf_impl.values_ref = new T *[CHUNK_SIZE];
                    std::fill(node->leaf_impl.values_ref,
                              node->leaf_impl.values_ref + CHUNK_SIZE,
                              nullptr);
                    return node;
                }

                T *&last() {
                    return values_ref[fill - 1];
                }

                const T *last() const {
                    return values_ref[fill - 1];
                }
            } leaf_impl;

            rrbt_node_t *clone() const {
                rrbt_node_t *node;
                if (type == type_t::node) {
                    node = node_t::empty_node();
                    node->node_impl.fill = node_impl.fill;
                    std::copy(node_impl.nodes_ref,
                              node_impl.nodes_ref + CHUNK_SIZE,
                              node->node_impl.nodes_ref);
                } else {
                    node = leaf_t::empty_leaf();
                    node->leaf_impl.fill = leaf_impl.fill;
                    std::copy(leaf_impl.values_ref,
                              leaf_impl.values_ref + CHUNK_SIZE,
                              node->leaf_impl.values_ref);
                }
                node->size = size;
                return node;
            }
        };
    }

    template<typename T, size_t CHUNK_SIZE_BITS = 2U>
    class rrbt_t {
    public:
        typedef uint64_t size_t;
        using node_t = rrbt::rrbt_node_t<T, CHUNK_SIZE_BITS>;

        static rrbt_t *empty() {
            return new rrbt_t();
        }

        static rrbt_t *fill_with_value(size_t n, T &&value) {
            rrbt_t *vec = empty();
            for (size_t i = 0; i < n; ++i)
                vec->push_back_transient(value);
            return vec;
        }

        const T &at(size_t index) const {
            if (index >= size())
                throw random_access_out_of_range(index, size());
            return get(index);
        }

        rrbt_t *push_back_copy(T &value) const {
            rrbt_t *new_tree = node_push_back(value, true);
            return new_tree;
        }

        rrbt_t *push_back_transient(T &value) {
            rrbt_t *new_tree = node_push_back(value, false);
            *this = *new_tree;
            return this;
        }

        [[nodiscard]] size_t size() const {
            return size_;
        }

    protected:
        size_t size_ = 0;
        size_t height = 0;
        node_t *root = nullptr;
        node_t *tail = nullptr;
        static const size_t CHUNK_SIZE = (1u << CHUNK_SIZE_BITS);

        const T &get(size_t index) const {
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

        static std::pair<rrbt_t *, node_t *> clone_path_to_tail_node(rrbt_t *rrbt, node_t *node) {
            auto *new_rrbt = new rrbt_t(*rrbt);
            node_t *new_node = nullptr;
            node_t *prev_root = new_rrbt->root;
            auto *cur = new_rrbt->root = new_rrbt->root->clone();
            if (prev_root != node) {
                while (true) {
                    bool stop = false;
                    if (cur->node_impl.last() == node)
                        stop = true;
                    cur = cur->node_impl.last() = cur->node_impl.last()->clone();
                    if (stop)
                        break;
                }
            }
            new_node = cur;
            if (rrbt->tail == node)
                new_rrbt->tail = new_node;
            return {new_rrbt, new_node};
        }

        static rrbt_t *append_new_tail(rrbt_t *rrbt, bool copying = true) {
            auto *new_tree = new rrbt_t(*rrbt);
            node_t *last_not_full = nullptr;
            size_t last_height = 0;
            auto *cur = rrbt->root;
            size_t cur_height = rrbt->height;
            while (cur->type != node_t::type_t::leaf) {
                if (cur->node_impl.fill < CHUNK_SIZE)
                    last_not_full = cur, last_height = cur_height;
                cur = cur->node_impl.last();
                cur_height--;
            }
            if (!last_not_full) {
                auto *prev_root = new_tree->root;
                new_tree->root = node_t::node_t::empty_node();
                new_tree->root->node_impl.nodes_ref[0] = prev_root;
                new_tree->root->node_impl.fill = 1;
                new_tree->root->size = prev_root->size;
                new_tree->height++;
                last_not_full = new_tree->root;
                last_height = new_tree->height;
            } else if (copying) {
                std::tie(new_tree, last_not_full) = clone_path_to_tail_node(new_tree, last_not_full);
            }

            cur = last_not_full;
            cur_height = last_height;
            while (cur->type != node_t::type_t::leaf) {
                cur->node_impl.fill++;
                if (cur_height - 1 == 1)
                    cur->node_impl.last() = node_t::leaf_t::empty_leaf();
                else
                    cur->node_impl.last() = node_t::node_t::empty_node();
                cur = cur->node_impl.last();
                cur_height--;
            }
            new_tree->tail = cur;

            return new_tree;
        }

        rrbt_t *node_push_back(T &value, bool copying) const {
//            std::cerr << size_ << "\n";
            auto *new_rrbt = new rrbt_t(*this);

            if (!new_rrbt->root) {
                new_rrbt->root = node_t::leaf_t::empty_leaf();
                new_rrbt->height = 1;
                new_rrbt->tail = new_rrbt->root;
            } else if (tail->size == CHUNK_SIZE) {
                new_rrbt = append_new_tail(new_rrbt, copying);
            } else if (copying) {
                new_rrbt = clone_path_to_tail_node(new_rrbt, new_rrbt->tail).first;
            }
            new_rrbt->size_++;

            new_rrbt->tail->size++;
            new_rrbt->tail->leaf_impl.fill++;
            new_rrbt->tail->leaf_impl.last() = new T(value);

            auto *cur = new_rrbt->root;
            while (cur->type != node_t::type_t::leaf) {
                cur->size++;
                cur = cur->node_impl.last();
            }

            return new_rrbt;
        }
    };
} //immu::details

#endif //IMMU_RRBT_H
