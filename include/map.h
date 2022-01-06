/* Copyright [2022] [Jonas Hegemann]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#ifndef MAP_H
#define MAP_H

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <functional>
#include <map>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

#include "json.h"
#include "log.h"

#undef INNER_BINARY_SEARCH
#undef OUTER_BINARY_SEARCH

#define INNER_FANOUT 32
#define OUTER_FANOUT 32

class Node;
template <class K, class V> class InnerNode;
template <class K, class V> class OuterNode;
template <class K, class V> class Map;
template <class K, class V> class MapIterator;
template <class K, class V> class Multimap;
template <class K, class V> class MultimapIterator;

template <class T> class Serializer;
template <> class Serializer<std::string>;
template <class K, class V> class Serializer<std::map<K, V>>;
template <class K> class Serializer<std::map<K, std::string>>;
template <class V> class Serializer<std::map<std::string, V>>;
template <class T> class Serializer<std::vector<T>>;
template <> class Serializer<std::vector<std::string>>;
template <> class Serializer<JsonObject>;
template <> class Serializer<JsonArray>;
template <class K, class V> class Serializer<Map<K, V>>;

class Node {
public:
  Node() {}
  virtual ~Node() {}
  virtual bool IsOuter() = 0;
  virtual bool IsSparse() = 0;
  virtual bool IsFull() = 0;
  virtual bool Redistribute(Node *node) = 0;
  virtual bool Coalesce(Node *node) = 0;
  virtual void SetParent(Node *node) = 0;
  virtual Node *GetParent() = 0;
};

template <class K, class V> class InnerNode : public Node {
  template <class> friend class ::Serializer;
  template <class, class> friend class ::OuterNode;
  template <class, class> friend class ::Map;
  template <class, class> friend class ::MapIterator;
  template <class, class> friend class ::Multimap;
  template <class, class> friend class ::MultimapIterator;

public:
  InnerNode();
  virtual ~InnerNode();
  Node *GetParent();
  void SetParent(Node *node);
  bool IsOuter();
  bool IsSparse();
  bool IsFull();
  size_t CountKeys();
  size_t CountChildren();
  K &Key(size_t index);
  const K &GetKey(size_t index) const;
  Node *Child(size_t index);
  const Node *GetChild(size_t index) const;
  size_t ChildIndex(const Node *child);
  size_t KeyIndex(const K &key);
  void Insert(Node *left, K &separator, Node *right);
  void Erase(const K &key, Node *child);
  std::tuple<InnerNode<K, V> *, K> Split();
  size_t SeparatorIndex(InnerNode<K, V> *sibling);
  bool Redistribute(Node *node);
  bool Coalesce(Node *node);

protected:
  Node *parent_;
  std::vector<K> keys_;
  std::vector<Node *> children_;
};

template <class K, class V> InnerNode<K, V>::InnerNode() {
  keys_.reserve(INNER_FANOUT + 1);
  children_.reserve(INNER_FANOUT + 2);
}

template <class K, class V> InnerNode<K, V>::~InnerNode() {}

template <class K, class V> inline Node *InnerNode<K, V>::GetParent() {
  return parent_;
}

template <class K, class V> inline void InnerNode<K, V>::SetParent(Node *node) {
  parent_ = node;
}

template <class K, class V> inline bool InnerNode<K, V>::IsOuter() {
  return false;
}

template <class K, class V> inline bool InnerNode<K, V>::IsSparse() {
  return keys_.size() < INNER_FANOUT / 2;
}

template <class K, class V> inline bool InnerNode<K, V>::IsFull() {
  return keys_.size() > INNER_FANOUT;
}

template <class K, class V> inline size_t InnerNode<K, V>::CountKeys() {
  return keys_.size();
}

template <class K, class V> inline size_t InnerNode<K, V>::CountChildren() {
  return children_.size();
}

template <class K, class V> inline K &InnerNode<K, V>::Key(size_t index) {
  return keys_[index];
}

template <class K, class V>
inline const K &InnerNode<K, V>::GetKey(size_t index) const {
  return keys_[index];
}

template <class K, class V> inline Node *InnerNode<K, V>::Child(size_t index) {
  return children_[index];
}

template <class K, class V>
inline const Node *InnerNode<K, V>::GetChild(size_t index) const {
  return children_[index];
}

template <class K, class V>
size_t InnerNode<K, V>::ChildIndex(const Node *child) {
  if (children_.empty()) {
    return std::string::npos;
  }
  const size_t size = children_.size();
  size_t position = 0;
  while (position < size && children_[position] != child) {
    position++;
  }
  if (position < size) {
    return position;
  }
  return std::string::npos;
}

template <class K, class V> size_t InnerNode<K, V>::KeyIndex(const K &key) {
#ifdef INNER_BINARY_SEARCH
  typename std::vector<K>::iterator it =
      lower_bound(keys_.begin(), keys_.end(), key);
  if (it != keys_.end() && !(key < *it)) {
    return it - keys_.begin();
  }
  return std::string::npos;
#else
  if (keys_.empty()) {
    return std::string::npos;
  }
  const size_t size = keys_.size();
  size_t position = 0;
  while (position < size && keys_[position] != key) {
    position++;
  }
  if (position < size) {
    return position;
  }
  return std::string::npos;
#endif
}

template <class K, class V>
void InnerNode<K, V>::Insert(Node *left, K &separator, Node *right) {
  left->SetParent(this);
  right->SetParent(this);
  if (keys_.empty()) {
    children_.push_back(left);
    children_.push_back(right);
    keys_.push_back(separator);
    return;
  }
  const size_t position = ChildIndex(left);
  if (position == std::string::npos) {
    abort();
  }
  keys_.insert(keys_.begin() + position, separator);
  children_.insert(children_.begin() + position + 1, right);
}

template <class K, class V>
void InnerNode<K, V>::Erase(const K &key, Node *child) {
  const size_t key_position = KeyIndex(key);
  if (key_position == std::string::npos) {
    return;
  }
  const size_t child_position = ChildIndex(child);
  if (child_position == std::string::npos) {
    return;
  }
  keys_.erase(keys_.begin() + key_position);
  children_.erase(children_.begin() + child_position);
}

template <class K, class V>
std::tuple<InnerNode<K, V> *, K> InnerNode<K, V>::Split() {
  const size_t size = keys_.size();
  const size_t keys_left = size / 2;
  const size_t children_left = keys_left + 1;
  InnerNode<K, V> *sibling = new InnerNode<K, V>();
  const K up_key = keys_[keys_left];
  std::move(keys_.begin() + keys_left + 1, keys_.end(),
            std::back_inserter(sibling->keys_));
  std::move(children_.begin() + children_left, children_.end(),
            std::back_inserter(sibling->children_));
  keys_.erase(keys_.begin() + keys_left, keys_.end());
  children_.erase(children_.begin() + children_left, children_.end());
  for (auto it = sibling->children_.begin(); it != sibling->children_.end();
       ++it) {
    (*it)->SetParent(sibling);
  }
  sibling->SetParent(parent_);
  return std::make_tuple(sibling, up_key);
}

template <class K, class V>
size_t InnerNode<K, V>::SeparatorIndex(InnerNode<K, V> *sibling) {
  const size_t self_index =
      static_cast<InnerNode<K, V> *>(parent_)->ChildIndex(this);
  const size_t sibling_index =
      static_cast<InnerNode<K, V> *>(parent_)->ChildIndex(sibling);
  return std::min(self_index, sibling_index);
}

template <class K, class V> bool InnerNode<K, V>::Redistribute(Node *node) {
  InnerNode<K, V> *sibling = static_cast<InnerNode<K, V> *>(node);
  const size_t separator_index = SeparatorIndex(sibling);
  K up_key = static_cast<InnerNode<K, V> *>(parent_)->keys_[separator_index];
  if (sibling->keys_.size() >= keys_.size() + 2) {
    keys_.push_back(up_key);
    children_.push_back(sibling->children_.front());
    sibling->children_.erase(sibling->children_.begin());
    children_.back()->SetParent(this);
    static_cast<InnerNode<K, V> *>(parent_)->keys_[separator_index] =
        sibling->keys_[0];
    sibling->keys_.erase(sibling->keys_.begin());
    return true;
  }
  if (keys_.size() >= sibling->keys_.size() + 2) {
    sibling->keys_.insert(sibling->keys_.begin(), up_key);
    sibling->children_.insert(sibling->children_.begin(), children_.back());
    children_.pop_back();
    sibling->children_.front()->SetParent(sibling);
    static_cast<InnerNode<K, V> *>(parent_)->keys_[separator_index] =
        keys_.back();
    keys_.pop_back();
    return true;
  }
  return false;
}

template <class K, class V> bool InnerNode<K, V>::Coalesce(Node *node) {
  InnerNode<K, V> *sibling = static_cast<InnerNode<K, V> *>(node);
  if (keys_.size() + sibling->keys_.size() > INNER_FANOUT) {
    return false;
  }
  const size_t separator_index = SeparatorIndex(sibling);
  const K up_key =
      static_cast<InnerNode<K, V> *>(parent_)->keys_[separator_index];
  keys_.push_back(up_key);
  std::move(sibling->keys_.begin(), sibling->keys_.end(),
            std::back_inserter(keys_));
  sibling->keys_.clear();
  for (auto it = sibling->children_.begin(); it != sibling->children_.end();
       ++it) {
    (*it)->SetParent(this);
  }
  std::move(sibling->children_.begin(), sibling->children_.end(),
            std::back_inserter(children_));
  sibling->children_.clear();
  return true;
}

template <class K, class V> class OuterNode : public Node {
  template <class> friend class ::Serializer;
  template <class, class> friend class ::InnerNode;
  template <class, class> friend class ::Map;
  template <class, class> friend class ::MapIterator;
  template <class, class> friend class ::Multimap;
  template <class, class> friend class ::MultimapIterator;

public:
  OuterNode();
  virtual ~OuterNode();
  Node *GetParent();
  void SetParent(Node *node);
  bool IsOuter();
  bool IsSparse();
  bool IsFull();
  size_t CountKeys();
  size_t CountValues();
  K &Key(size_t index);
  const K &GetKey(size_t index) const;
  V &Value(size_t index);
  const V &GetValue(size_t index) const;
  size_t ValueIndex(const V &value);
  size_t KeyIndex(const K &key);
  void Insert(const K &key, const V &value);
  void Erase(const K &key);
  std::tuple<OuterNode<K, V> *, K> Split();
  bool Redistribute(Node *node);
  bool Coalesce(Node *node);
  OuterNode<K, V> *GetNext();
  OuterNode<K, V> *GetPrevious();

protected:
  std::vector<K> keys_;
  std::vector<V> values_;
  Node *parent_;
  OuterNode<K, V> *next_;
  OuterNode<K, V> *previous_;
};

template <class K, class V>
OuterNode<K, V>::OuterNode() : next_(nullptr), previous_(nullptr) {
  keys_.reserve(OUTER_FANOUT + 1);
  values_.reserve(OUTER_FANOUT + 1);
}

template <class K, class V> OuterNode<K, V>::~OuterNode() {}

template <class K, class V> inline Node *OuterNode<K, V>::GetParent() {
  return parent_;
}

template <class K, class V> inline void OuterNode<K, V>::SetParent(Node *node) {
  parent_ = node;
}

template <class K, class V> inline bool OuterNode<K, V>::IsOuter() {
  return true;
}

template <class K, class V> inline bool OuterNode<K, V>::IsSparse() {
  return keys_.size() < OUTER_FANOUT / 2;
}

template <class K, class V> inline bool OuterNode<K, V>::IsFull() {
  return keys_.size() > OUTER_FANOUT;
}

template <class K, class V> inline size_t OuterNode<K, V>::CountKeys() {
  return keys_.size();
}

template <class K, class V> inline size_t OuterNode<K, V>::CountValues() {
  return values_.size();
}

template <class K, class V> inline K &OuterNode<K, V>::Key(size_t index) {
  return keys_[index];
}

template <class K, class V>
inline const K &OuterNode<K, V>::GetKey(size_t index) const {
  return keys_[index];
}

template <class K, class V> inline V &OuterNode<K, V>::Value(size_t index) {
  return values_[index];
}

template <class K, class V>
inline const V &OuterNode<K, V>::GetValue(size_t index) const {
  return values_[index];
}

template <class K, class V> size_t OuterNode<K, V>::ValueIndex(const V &value) {
  if (values_.empty()) {
    return std::string::npos;
  }
  const size_t size = values_.size();
  size_t position = 0;
  while (position < size && values_[position] != value) {
    position++;
  }
  if (position < size) {
    return position;
  }
  return std::string::npos;
}

template <class K, class V> size_t OuterNode<K, V>::KeyIndex(const K &key) {
#ifdef OUTER_BINARY_SEARCH
  typename std::vector<K>::iterator it =
      lower_bound(keys_.begin(), keys_.end(), key);
  if (it != keys_.end() && !(key < *it)) {
    return it - keys_.begin();
  }
  return std::string::npos;
#else
  if (keys_.empty()) {
    return std::string::npos;
  }
  const size_t size = keys_.size();
  size_t position = 0;
  while (position < size && keys_[position] != key) {
    position++;
  }
  if (position < size) {
    return position;
  }
  return std::string::npos;
#endif
}

template <class K, class V>
void OuterNode<K, V>::Insert(const K &key, const V &value) {
  if (keys_.empty()) {
    keys_.push_back(key);
    values_.push_back(value);
    return;
  }
  const size_t size = keys_.size();
  size_t position = 0;
  while (position < size && keys_[position] < key) {
    position++;
  }
  keys_.insert(keys_.begin() + position, key);
  values_.insert(values_.begin() + position, value);
}

template <class K, class V> void OuterNode<K, V>::Erase(const K &key) {
  const size_t key_position = KeyIndex(key);
  if (key_position == std::string::npos) {
    return;
  }
  size_t value_position = key_position;
  keys_.erase(keys_.begin() + key_position);
  values_.erase(values_.begin() + value_position);
}

template <class K, class V>
std::tuple<OuterNode<K, V> *, K> OuterNode<K, V>::Split() {
  const size_t size = keys_.size();
  const size_t keys_left = (size % 2 == 0) ? size / 2 : size / 2 + 1;
  OuterNode<K, V> *sibling = new OuterNode<K, V>();
  std::move(keys_.begin() + keys_left, keys_.end(),
            std::back_inserter(sibling->keys_));
  std::move(values_.begin() + keys_left, values_.end(),
            std::back_inserter(sibling->values_));
  keys_.erase(keys_.begin() + keys_left, keys_.end());
  values_.erase(values_.begin() + keys_left, values_.end());
  const K up_key = sibling->keys_.front();
  sibling->next_ = next_;
  sibling->previous_ = this;
  if (next_ != nullptr) {
    next_->previous_ = sibling;
  }
  next_ = sibling;
  sibling->parent_ = parent_;
  return std::make_tuple(sibling, up_key);
}

template <class K, class V> bool OuterNode<K, V>::Redistribute(Node *node) {
  OuterNode<K, V> *sibling = static_cast<OuterNode<K, V> *>(node);
  if (sibling->keys_.size() >= keys_.size() + 2) {
    keys_.push_back(sibling->keys_.front());
    values_.push_back(sibling->values_.front());
    sibling->keys_.erase(sibling->keys_.begin());
    sibling->values_.erase(sibling->values_.begin());
  } else if (keys_.size() >= sibling->keys_.size() + 2) {
    sibling->keys_.insert(sibling->keys_.begin(), keys_.back());
    sibling->values_.insert(sibling->values_.begin(), values_.back());
    keys_.pop_back();
    values_.pop_back();
  } else {
    return false;
  }
  const K up_key = sibling->keys_.front();
  const size_t up_key_index =
      static_cast<InnerNode<K, V> *>(parent_)->ChildIndex(this);
  static_cast<InnerNode<K, V> *>(parent_)->keys_[up_key_index] = up_key;
  return true;
}

template <class K, class V> bool OuterNode<K, V>::Coalesce(Node *node) {
  OuterNode<K, V> *sibling = static_cast<OuterNode<K, V> *>(node);
  if (sibling->keys_.size() + keys_.size() > OUTER_FANOUT) {
    return false;
  }
  std::move(sibling->keys_.begin(), sibling->keys_.end(),
            std::back_inserter(keys_));
  std::move(sibling->values_.begin(), sibling->values_.end(),
            std::back_inserter(values_));
  sibling->keys_.clear();
  sibling->values_.clear();
  next_ = sibling->next_;
  if (next_ != nullptr) {
    next_->previous_ = this;
  }
  return true;
}

template <class K, class V> inline OuterNode<K, V> *OuterNode<K, V>::GetNext() {
  return next_;
}

template <class K, class V>
inline OuterNode<K, V> *OuterNode<K, V>::GetPrevious() {
  return previous_;
}

template <class K, class V> class Map {
  template <class> friend class ::Serializer;
  template <class, class> friend class ::InnerNode;
  template <class, class> friend class ::OuterNode;
  template <class, class> friend class ::MapIterator;
  template <class, class> friend class ::Multimap;
  template <class, class> friend class ::MultimapIterator;

public:
  Map();
  virtual ~Map();
  void Clear();
  size_t Size();
  void Insert(const K &key, const V &value);
  void Insert(MapIterator<K, V> &iter, const V &value);
  const V &operator[](const K &key) const;
  V &operator[](const K &key);
  bool Erase(const K &key);
  bool Erase(MapIterator<K, V> iter);
  bool Contains(const K &key);
  MapIterator<K, V> Find(const K &key);
  MapIterator<K, V> Begin();
  const MapIterator<K, V> Begin() const;
  MapIterator<K, V> End();
  const MapIterator<K, V> End() const;

protected:
  Node *root_;
  size_t size_;
  V &Get(const K &key);
  bool Erase(OuterNode<K, V> *outer, const K &key);
  Node *LeftNode(Node *node);
  Node *RightNode(Node *node);
  size_t SeparatorIndex(Node *node, Node *sibling);
  K SeparatorKey(Node *node, Node *sibling);
  void PropagateUpwards(Node *origin, K &up_key, Node *sibling);
  std::tuple<size_t, OuterNode<K, V> *> Locate(const K &key);
  MapIterator<K, V> BeginIterator();
  OuterNode<K, V> *FirstLeaf();
  OuterNode<K, V> *LastLeaf();
};

template <class K, class V> Map<K, V>::Map() : root_(nullptr), size_(0) {}

template <class K, class V> Map<K, V>::~Map() { Clear(); }

template <class K, class V> void Map<K, V>::Clear() {
  if (root_ != nullptr) {
    std::stack<Node *> todo;
    todo.push(root_);
    Node *current;
    InnerNode<K, V> *inner_node;
    while (!todo.empty()) {
      current = std::move(todo.top());
      todo.pop();
      if (!current->IsOuter()) {
        inner_node = static_cast<InnerNode<K, V> *>(current);
        for (auto it = inner_node->children_.begin();
             it != inner_node->children_.end(); ++it) {
          todo.push(*it);
        }
      }
      delete current;
    }
  }
  root_ = nullptr;
  size_ = 0;
}

template <class K, class V> size_t Map<K, V>::Size() { return size_; }

template <class K, class V> Node *Map<K, V>::LeftNode(Node *node) {
  if (node == root_) {
    return nullptr;
  }
  InnerNode<K, V> *node_parent =
      static_cast<InnerNode<K, V> *>(node->GetParent());
  const size_t position = node_parent->ChildIndex(node);
  const size_t left_index = (position == 0) ? std::string::npos : position - 1;
  if (left_index != std::string::npos) {
    return node_parent->children_[left_index];
  }
  return nullptr;
}

template <class K, class V> Node *Map<K, V>::RightNode(Node *node) {
  if (node == root_) {
    return nullptr;
  }
  InnerNode<K, V> *node_parent =
      static_cast<InnerNode<K, V> *>(node->GetParent());
  const size_t position = node_parent->ChildIndex(node);
  const size_t right_index = (position == node_parent->children_.size() - 1)
                                 ? std::string::npos
                                 : position + 1;
  if (right_index != std::string::npos) {
    return node_parent->children_[right_index];
  }
  return nullptr;
}

template <class K, class V>
size_t Map<K, V>::SeparatorIndex(Node *node, Node *sibling) {
  InnerNode<K, V> *parent = static_cast<InnerNode<K, V> *>(node->GetParent());
  const size_t node_position = parent->ChildIndex(node);
  const size_t sibling_position = parent->ChildIndex(sibling);
  return std::min(node_position, sibling_position);
}

template <class K, class V>
K Map<K, V>::SeparatorKey(Node *node, Node *sibling) {
  const size_t index = SeparatorIndex(node, sibling);
  InnerNode<K, V> *parent = static_cast<InnerNode<K, V> *>(node->GetParent());
  return parent->keys_[index];
}

template <class K, class V>
void Map<K, V>::PropagateUpwards(Node *origin, K &up_key, Node *sibling) {
  if (origin == root_) {
    InnerNode<K, V> *inner_node = new InnerNode<K, V>();
    inner_node->Insert(origin, up_key, sibling);
    root_ = inner_node;
    return;
  }
  InnerNode<K, V> *next_origin =
      static_cast<InnerNode<K, V> *>(origin->GetParent());
  next_origin->Insert(origin, up_key, sibling);
  if (next_origin->IsFull()) {
    Node *next_sibling;
    K next_key;
    std::tie(next_sibling, next_key) = next_origin->Split();
    PropagateUpwards(next_origin, next_key, next_sibling);
  }
}

template <class K, class V>
std::tuple<size_t, OuterNode<K, V> *> Map<K, V>::Locate(const K &key) {
  Node *current = root_;
  if (current == nullptr) {
    return std::make_tuple(std::string::npos,
                           static_cast<OuterNode<K, V> *>(nullptr));
  }
  while (!current->IsOuter()) {
    InnerNode<K, V> *inner_node = static_cast<InnerNode<K, V> *>(current);
    if (key < inner_node->keys_.front()) {
      current = inner_node->children_.front();
      continue;
    }
    if (key >= inner_node->keys_.back()) {
      current = inner_node->children_.back();
      continue;
    }
    const size_t max_index = inner_node->keys_.size() - 1;
    for (size_t i = 0; i < max_index; i++) {
      if (inner_node->keys_[i] <= key && key < inner_node->keys_[i + 1]) {
        current = inner_node->children_[i + 1];
        break;
      }
    }
  }
  OuterNode<K, V> *outer_node = static_cast<OuterNode<K, V> *>(current);
  const size_t key_position = outer_node->KeyIndex(key);
  return std::make_tuple(key_position, outer_node);
}

template <class K, class V> OuterNode<K, V> *Map<K, V>::FirstLeaf() {
  if (root_ == nullptr) {
    return nullptr;
  }
  Node *current = root_;
  while (!current->IsOuter()) {
    current = static_cast<InnerNode<K, V> *>(current)->children_.front();
  }
  return static_cast<OuterNode<K, V> *>(current);
}

template <class K, class V> OuterNode<K, V> *Map<K, V>::LastLeaf() {
  if (root_ == nullptr) {
    return nullptr;
  }
  Node *current = root_;
  while (!current->IsOuter()) {
    InnerNode<K, V> *inner_node = static_cast<InnerNode<K, V> *>(current);
    current = inner_node->children_.back();
  }
  return static_cast<OuterNode<K, V> *>(current);
}

template <class K, class V> V &Map<K, V>::Get(const K &key) {
  size_t position;
  OuterNode<K, V> *outer_node;
  std::tie(position, outer_node) = Locate(key);
  return outer_node->values_[position];
}

template <class K, class V> const V &Map<K, V>::operator[](const K &key) const {
  return Get(key);
}

template <class K, class V> V &Map<K, V>::operator[](const K &key) {
  return Get(key);
}

template <class K, class V>
void Map<K, V>::Insert(MapIterator<K, V> &iter, const V &value) {
  if (iter == End()) {
    return;
  }
  iter.GetNode()->values_[iter.GetIndex()] = value;
}

template <class K, class V>
void Map<K, V>::Insert(const K &key, const V &value) {
  if (root_ == nullptr) {
    OuterNode<K, V> *outer_node = new OuterNode<K, V>();
    outer_node->Insert(key, value);
    root_ = outer_node;
    size_++;
    return;
  }
  size_t position = std::string::npos;
  OuterNode<K, V> *outer_node = nullptr;
  std::tie(position, outer_node) = Locate(key);
  if (position != std::string::npos) {
    outer_node->values_[position] = value;
    return;
  }
  outer_node->Insert(key, value);
  if (outer_node->IsFull()) {
    OuterNode<K, V> *sibling;
    K up_key;
    std::tie(sibling, up_key) = outer_node->Split();
    PropagateUpwards(outer_node, up_key, sibling);
  }
  size_++;
  return;
}

template <class K, class V>
bool Map<K, V>::Erase(OuterNode<K, V> *outer_node, const K &key) {
  outer_node->Erase(key);
  Node *current = outer_node;
  if (current == root_) {
    if (root_->IsOuter()) {
      if (static_cast<OuterNode<K, V> *>(root_)->CountKeys() == 0) {
        delete root_;
        root_ = nullptr;
      }
    }
    return true;
  }
  while (current != root_) {
    if (!current->IsSparse()) {
      return true;
    }
    Node *left = LeftNode(current);
    if (left != nullptr && left->Redistribute(current)) {
      return true;
    }
    Node *right = RightNode(current);
    if (right != nullptr && current->Redistribute(right)) {
      return true;
    }
    if (left != nullptr && left->Coalesce(current)) {
      InnerNode<K, V> *parent =
          static_cast<InnerNode<K, V> *>(current->GetParent());
      const K separator_key = SeparatorKey(left, current);
      parent->Erase(separator_key, current);
      Node *backup = current;
      current = current->GetParent();
      delete backup;
      continue;
    }
    if (right != nullptr && current->Coalesce(right)) {
      InnerNode<K, V> *parent =
          static_cast<InnerNode<K, V> *>(current->GetParent());
      const K separator_key = SeparatorKey(current, right);
      parent->Erase(separator_key, right);
      Node *backup = right;
      current = current->GetParent();
      delete backup;
      continue;
    }
  }
  InnerNode<K, V> *inner_node = static_cast<InnerNode<K, V> *>(current);
  if (inner_node->keys_.size() == 0) {
    Node *backup = root_;
    root_ = inner_node->children_.front();
    root_->SetParent(nullptr);
    delete backup;
  }
  return true;
}

template <class K, class V> bool Map<K, V>::Erase(const K &key) {
  size_t position;
  OuterNode<K, V> *outer_node;
  std::tie(position, outer_node) = Locate(key);
  if (position == std::string::npos) {
    return false;
  }
  size_--;
  return Erase(outer_node, key);
}

template <class K, class V> bool Map<K, V>::Erase(MapIterator<K, V> iter) {
  size_--;
  return Erase(iter.GetNode(), iter.GetKey());
}

template <class K, class V> bool Map<K, V>::Contains(const K &key) {
  size_t position;
  OuterNode<K, V> *outer_node;
  std::tie(position, outer_node) = Locate(key);
  if (position == std::string::npos) {
    return false;
  }
  return true;
}

template <class K, class V> MapIterator<K, V> Map<K, V>::Find(const K &key) {
  MapIterator<K, V> iter;
  size_t index = std::string::npos;
  OuterNode<K, V> *outer_node = nullptr;
  std::tie(index, outer_node) = Locate(key);
  if (index != std::string::npos) {
    iter.index_ = index;
    iter.node_ = outer_node;
  }
  return iter;
}

template <class K, class V> MapIterator<K, V> Map<K, V>::BeginIterator() {
  if (root_ == nullptr) {
    return End();
  }
  MapIterator<K, V> iter;
  iter.node_ = FirstLeaf();
  iter.index_ = 0;
  return iter;
}

template <class K, class V> MapIterator<K, V> Map<K, V>::Begin() {
  return BeginIterator();
}

template <class K, class V> const MapIterator<K, V> Map<K, V>::Begin() const {
  return BeginIterator();
}

template <class K, class V> MapIterator<K, V> Map<K, V>::End() {
  return MapIterator<K, V>();
}

template <class K, class V> const MapIterator<K, V> Map<K, V>::End() const {
  return MapIterator<K, V>();
}

template <class K, class V> class MapIterator {
  template <class> friend class ::Serializer;
  template <class, class> friend class ::InnerNode;
  template <class, class> friend class ::OuterNode;
  template <class, class> friend class ::Map;
  template <class, class> friend class ::Multimap;
  template <class, class> friend class ::MultimapIterator;

public:
  MapIterator();
  virtual ~MapIterator();
  const K &GetKey() const;
  const V &GetValue() const;
  MapIterator<K, V> operator++();
  MapIterator<K, V> operator++(int);
  MapIterator<K, V> operator--();
  MapIterator<K, V> operator--(int);
  bool operator==(const MapIterator<K, V> &rhs);
  bool operator!=(const MapIterator<K, V> &rhs);

protected:
  size_t GetIndex();
  OuterNode<K, V> *GetNode();
  K &Key();
  V &Value();
  OuterNode<K, V> *node_;
  size_t index_;
  void Increment();
  void Decrement();
};

template <class K, class V>
MapIterator<K, V>::MapIterator() : node_(nullptr), index_(std::string::npos) {}

template <class K, class V> MapIterator<K, V>::~MapIterator() {}

template <class K, class V> inline K &MapIterator<K, V>::Key() {
  return node_->Key(index_);
}

template <class K, class V> inline const K &MapIterator<K, V>::GetKey() const {
  return node_->GetKey(index_);
}

template <class K, class V> inline V &MapIterator<K, V>::Value() {
  return node_->Value(index_);
}

template <class K, class V>
inline const V &MapIterator<K, V>::GetValue() const {
  return node_->GetValue(index_);
}

template <class K, class V> inline size_t MapIterator<K, V>::GetIndex() {
  return index_;
}

template <class K, class V>
inline OuterNode<K, V> *MapIterator<K, V>::GetNode() {
  return node_;
}

template <class K, class V>
inline MapIterator<K, V> MapIterator<K, V>::operator++() {
  Increment();
  return *this;
}

template <class K, class V>
inline MapIterator<K, V> MapIterator<K, V>::operator++(int) {
  MapIterator<K, V> temp = *this;
  Increment();
  return temp;
}

template <class K, class V>
inline MapIterator<K, V> MapIterator<K, V>::operator--() {
  Decrement();
  return *this;
}

template <class K, class V>
inline MapIterator<K, V> MapIterator<K, V>::operator--(int) {
  MapIterator<K, V> temp = *this;
  Decrement();
  return temp;
}

template <class K, class V>
inline bool MapIterator<K, V>::operator==(const MapIterator<K, V> &rhs) {
  return node_ == rhs.node_ && index_ == rhs.index_;
}

template <class K, class V>
inline bool MapIterator<K, V>::operator!=(const MapIterator<K, V> &rhs) {
  return !(*this == rhs);
}

template <class K, class V> void MapIterator<K, V>::Increment() {
  if (index_ == node_->CountKeys() - 1) {
    if (node_->GetNext() != nullptr) {
      node_ = node_->GetNext();
      index_ = 0;
    } else {
      node_ = nullptr;
      index_ = std::string::npos;
    }
  } else {
    index_++;
  }
}

template <class K, class V> void MapIterator<K, V>::Decrement() {
  if (index_ == 0) {
    if (node_->GetPrevious() != nullptr) {
      node_ = node_->GetPrevious();
      index_ = node_->CountKeys() - 1;
    } else {
      node_ = nullptr;
      index_ = std::string::npos;
    }
  } else {
    index_--;
  }
}

template <class K, class V> class Multimap {
  template <class> friend class ::Serializer;
  template <class, class> friend class ::InnerNode;
  template <class, class> friend class ::OuterNode;
  template <class, class> friend class ::Map;
  template <class, class> friend class ::MapIterator;
  template <class, class> friend class ::MultimapIterator;

public:
  Multimap();
  virtual ~Multimap();
  void Insert(const K &key, const V &value);
  void Insert(MultimapIterator<K, V> &iter, const V &value);
  const std::vector<V> &operator[](const K &key) const;
  std::vector<V> &operator[](const K &key);
  void Clear();
  bool Erase(const K &key);
  bool Erase(const K &key, const V &value);
  bool Erase(MultimapIterator<K, V> iter);
  bool Contains(const K &key);
  MultimapIterator<K, V> Find(const K &key);
  MultimapIterator<K, V> Begin();
  const MultimapIterator<K, V> Begin() const;
  MultimapIterator<K, V> End();
  const MultimapIterator<K, V> End() const;

protected:
  Map<K, std::vector<V>> tree_;
  std::vector<V> &Get(const K &key);
  MultimapIterator<K, V> BeginIterator();
};

template <class K, class V> Multimap<K, V>::Multimap() {}

template <class K, class V> Multimap<K, V>::~Multimap() {}

template <class K, class V>
void Multimap<K, V>::Insert(const K &key, const V &value) {
  MapIterator<K, std::vector<V>> iter = tree_.Find(key);
  if (iter != tree_.End()) {
    std::vector<V> &multi_value = iter.Value();
    multi_value.push_back(value);
    return;
  }
  tree_.Insert(key, std::vector<V>{value});
  return;
}

template <class K, class V>
void Multimap<K, V>::Insert(MultimapIterator<K, V> &iter, const V &value) {
  MapIterator<K, std::vector<V>> single_iter;
  single_iter.node_ = iter.node_;
  single_iter.index_ = iter.index_;
  std::vector<V> &multi_value = single_iter.Value();
  multi_value[iter.multi_index_] = value;
}

template <class K, class V>
inline std::vector<V> &Multimap<K, V>::Get(const K &key) {
  return tree_.Get(key);
}

template <class K, class V> inline void Multimap<K, V>::Clear() {
  tree_.Clear();
}

template <class K, class V>
const std::vector<V> &Multimap<K, V>::operator[](const K &key) const {
  return Get(key);
}

template <class K, class V>
std::vector<V> &Multimap<K, V>::operator[](const K &key) {
  return Get(key);
}

template <class K, class V> inline bool Multimap<K, V>::Erase(const K &key) {
  return tree_.Erase(key);
}

template <class K, class V>
bool Multimap<K, V>::Erase(const K &key, const V &value) {
  MapIterator<K, std::vector<V>> iter = tree_.Find(key);
  if (iter != tree_.End()) {
    std::vector<V> &multi_value = iter.Value();
    if (multi_value.size() == 1) {
      tree_.Erase(key);
      return true;
    }
    for (size_t i = 0; i < multi_value.size(); i++) {
      if (multi_value.at(i) == value) {
        multi_value.erase(multi_value.begin() + i);
        return true;
      }
    }
  }
  return false;
}

template <class K, class V>
inline bool Multimap<K, V>::Erase(MultimapIterator<K, V> iter) {
  return Erase(iter.GetKey(), iter.GetValue());
}

template <class K, class V> inline bool Multimap<K, V>::Contains(const K &key) {
  return tree_.Contains(key);
}

template <class K, class V>
MultimapIterator<K, V> Multimap<K, V>::Find(const K &key) {
  MapIterator<K, std::vector<V>> iter = tree_.Find(key);
  MultimapIterator<K, V> multi_iter;
  if (iter != tree_.End()) {
    multi_iter.index_ = iter.GetIndex();
    multi_iter.multi_index_ = 0;
    multi_iter.node_ = iter.GetNode();
  }
  return multi_iter;
}

template <class K, class V>
MultimapIterator<K, V> Multimap<K, V>::BeginIterator() {
  MultimapIterator<K, V> iter;
  iter.node_ = tree_.Begin().GetNode();
  if (iter.node_ == nullptr) {
    iter.index_ = std::string::npos;
    iter.multi_index_ = std::string::npos;
  } else {
    iter.index_ = 0;
    iter.multi_index_ = 0;
  }
  return iter;
}

template <class K, class V>
inline MultimapIterator<K, V> Multimap<K, V>::Begin() {
  return BeginIterator();
}

template <class K, class V>
const MultimapIterator<K, V> inline Multimap<K, V>::Begin() const {
  return BeginIterator();
}

template <class K, class V>
inline MultimapIterator<K, V> Multimap<K, V>::End() {
  return MultimapIterator<K, V>();
}

template <class K, class V>
const MultimapIterator<K, V> inline Multimap<K, V>::End() const {
  return MultimapIterator<K, V>();
}

template <class K, class V> class MultimapIterator {
  template <class> friend class ::Serializer;
  template <class, class> friend class ::InnerNode;
  template <class, class> friend class ::OuterNode;
  template <class, class> friend class ::Map;
  template <class, class> friend class ::MapIterator;
  template <class, class> friend class ::Multimap;

public:
  MultimapIterator();
  virtual ~MultimapIterator();
  const K &GetKey() const;
  const V &GetValue() const;
  const std::vector<V> &GetMultiValue() const;
  MultimapIterator<K, V> operator++();
  MultimapIterator<K, V> operator++(int);
  MultimapIterator<K, V> operator--();
  MultimapIterator<K, V> operator--(int);
  bool operator==(const MultimapIterator<K, V> &rhs);
  bool operator!=(const MultimapIterator<K, V> &rhs);

protected:
  K &Key();
  V &Value();
  std::vector<V> &MultiValue();
  OuterNode<K, std::vector<V>> *node_;
  size_t index_;
  size_t multi_index_;
  void Increment();
  void Decrement();
};

template <class K, class V>
MultimapIterator<K, V>::MultimapIterator()
    : node_(nullptr), index_(std::string::npos),
      multi_index_(std::string::npos) {}

template <class K, class V> MultimapIterator<K, V>::~MultimapIterator() {}

template <class K, class V> inline K &MultimapIterator<K, V>::Key() {
  return node_->GetKey(index_);
}

template <class K, class V>
inline const K &MultimapIterator<K, V>::GetKey() const {
  return node_->GetKey(index_);
}

template <class K, class V> inline V &MultimapIterator<K, V>::Value() {
  return node_->GetValue(index_)[multi_index_];
}

template <class K, class V>
inline const V &MultimapIterator<K, V>::GetValue() const {
  return node_->GetValue(index_)[multi_index_];
}

template <class K, class V>
inline std::vector<V> &MultimapIterator<K, V>::MultiValue() {
  return node_->GetValue(index_);
}

template <class K, class V>
inline const std::vector<V> &MultimapIterator<K, V>::GetMultiValue() const {
  return node_->GetValue(index_);
}

template <class K, class V>
inline MultimapIterator<K, V> MultimapIterator<K, V>::operator++() {
  Increment();
  return *this;
}

template <class K, class V>
inline MultimapIterator<K, V> MultimapIterator<K, V>::operator++(int) {
  MultimapIterator<K, V> temp = *this;
  Increment();
  return temp;
}

template <class K, class V>
inline MultimapIterator<K, V> MultimapIterator<K, V>::operator--() {
  Decrement();
  return *this;
}

template <class K, class V>
inline MultimapIterator<K, V> MultimapIterator<K, V>::operator--(int) {
  MultimapIterator<K, V> temp = *this;
  Decrement();
  return temp;
}

template <class K, class V>
inline bool
MultimapIterator<K, V>::operator==(const MultimapIterator<K, V> &rhs) {
  return node_ == rhs.node_ && index_ == rhs.index_ &&
         multi_index_ == rhs.multi_index_;
}

template <class K, class V>
inline bool
MultimapIterator<K, V>::operator!=(const MultimapIterator<K, V> &rhs) {
  return !(*this == rhs);
}

template <class K, class V> void MultimapIterator<K, V>::Increment() {
  if (index_ == node_->CountKeys() - 1) {
    if (multi_index_ == node_->GetValue(index_).size() - 1) {
      if (node_->GetNext() != nullptr) {
        node_ = node_->GetNext();
        index_ = 0;
        multi_index_ = 0;
      } else {
        node_ = nullptr;
        index_ = std::string::npos;
        multi_index_ = std::string::npos;
      }
    } else {
      multi_index_++;
    }
  } else {
    if (multi_index_ == node_->GetValue(index_).size() - 1) {
      index_++;
      multi_index_ = 0;
    } else {
      multi_index_++;
    }
  }
}

template <class K, class V> void MultimapIterator<K, V>::Decrement() {
  if (index_ == 0) {
    if (multi_index_ == 0) {
      if (node_->previous_ != nullptr) {
        node_ = node_->GetPrevious();
        index_ = node_->CountKeys() - 1;
        multi_index_ = node_->GetValue(index_).size() - 1;
      } else {
        node_ = nullptr;
        index_ = std::string::npos;
        multi_index_ = std::string::npos;
      }
    } else {
      multi_index_--;
    }
  } else {
    if (multi_index_ == 0) {
      index_--;
      multi_index_ = node_->GetValue(index_).size() - 1;
    } else {
      multi_index_--;
    }
  }
}

template <class T> class Serializer {
public:
  size_t Serialize(T &object, std::ostream &stream) {
    stream.write((const char *)&object, sizeof(T));
    return sizeof(T);
  }
  size_t Deserialize(T &object, std::istream &stream) {
    stream.read((char *)&object, sizeof(T));
    return sizeof(T);
  }
};

template <> class Serializer<std::string> {
public:
  size_t Serialize(std::string &object, std::ostream &stream) {
    size_t length = object.length();
    stream.write((const char *)&length, sizeof(size_t));
    stream.write((const char *)&object[0], length);
    return sizeof(size_t) + length;
  }
  size_t Deserialize(std::string &object, std::istream &stream) {
    object.clear();
    size_t length;
    stream.read((char *)&length, sizeof(size_t));
    object.resize(length);
    stream.read((char *)&object[0], length);
    return sizeof(size_t) + length;
  }
};

template <class K, class V> class Serializer<std::map<K, V>> {
public:
  size_t Serialize(std::map<K, V> &object, std::ostream &stream) {
    typename std::map<K, V>::iterator it;
    size_t size = object.size();
    stream.write((const char *)&size, sizeof(size_t));
    for (it = object.begin(); it != object.end(); ++it) {
      stream.write((const char *)&it->first, sizeof(K));
      stream.write((const char *)&it->second, sizeof(V));
    }
    return sizeof(size_t) + size * (sizeof(K) + sizeof(V));
  }
  size_t Deserialize(std::map<K, V> &object, std::istream &stream) {
    object.clear();
    size_t size;
    stream.read((char *)&size, sizeof(size_t));
    K key;
    V value;
    for (size_t i = 0; i < size; i++) {
      stream.read((char *)&key, sizeof(K));
      stream.read((char *)&value, sizeof(V));
      object.insert(key, value);
    }
    return sizeof(size_t) + size * (sizeof(K) + sizeof(V));
  }
};

template <class K> class Serializer<std::map<K, std::string>> {
public:
  size_t Serialize(std::map<K, std::string> &object, std::ostream &stream) {
    size_t result = 0;
    size_t size = object.size();
    stream.write((const char *)&size, sizeof(size_t));
    result += sizeof(size_t);
    for (auto it = object.begin(); it != object.end(); ++it) {
      stream.write((const char *)&it->first, sizeof(K));
      size_t length = it->second.length();
      stream.write((const char *)&it->second[0], length);
      result += sizeof(size_t) + length;
    }
    result += size * sizeof(K);
    return result;
  }
  size_t Deserialize(std::map<K, std::string> &object, std::istream &stream) {
    object.clear();
    size_t result = 0;
    size_t size;
    stream.read((char *)&size, sizeof(size_t));
    K key;
    std::string value;
    for (size_t i = 0; i < size; i++) {
      stream.read((char *)&key, sizeof(K));
      size_t length;
      stream.read((char *)&length, sizeof(size_t));
      value.resize(length);
      stream.read((char *)&value[0], length);
      result += sizeof(size_t) + length;
    }
    result += size * sizeof(K);
    return result;
  }
};

template <class V> class Serializer<std::map<std::string, V>> {
public:
  size_t Serialize(std::map<std::string, V> &object, std::ostream &stream) {
    size_t result = 0;
    size_t size = object.size();
    stream.write((const char *)&size, sizeof(size_t));
    result += sizeof(size_t);
    for (auto it = object.begin(); it != object.end(); ++it) {
      size_t length = it->first.length();
      stream.write((const char *)&it->first[0], length);
      result += sizeof(size_t) + length;
      stream.write((const char *)&it->second, sizeof(V));
    }
    result += size * sizeof(V);
    return result;
  }
  size_t Deserialize(std::map<std::string, V> &object, std::istream &stream) {
    object.clear();
    size_t result = 0;
    size_t size;
    stream.read((char *)&size, sizeof(size_t));
    std::string key;
    V value;
    for (size_t i = 0; i < size; i++) {
      size_t length;
      stream.read((char *)&length, sizeof(size_t));
      key.resize(length);
      stream.read((char *)&key[0], length);
      result += sizeof(size_t) + length;
      stream.read((char *)&value, sizeof(V));
    }
    result += size * sizeof(V);
    return result;
  }
};

template <class T> class Serializer<std::vector<T>> {
public:
  size_t Serialize(std::vector<T> &object, std::ostream &stream) {
    size_t size = object.size();
    stream.write((const char *)&size, sizeof(size_t));
    for (size_t i = 0; i < size; i++) {
      stream.write((const char *)&object[i], sizeof(T));
    }
    return sizeof(size_t) + size * sizeof(T);
  }
  size_t Deserialize(std::vector<T> &object, std::istream &stream) {
    object.clear();
    size_t size;
    stream.read((char *)&size, sizeof(size_t));
    object.resize(size);
    for (size_t i = 0; i < size; i++) {
      stream.read((char *)&object[i], sizeof(T));
    }
    return sizeof(size_t) + size * sizeof(T);
  }
};

template <> class Serializer<std::vector<std::string>> {
public:
  size_t Serialize(std::vector<std::string> &object, std::ostream &stream) {
    size_t result = 0;
    size_t size = object.size();
    stream.write((const char *)&size, sizeof(size_t));
    result += sizeof(size_t);
    for (size_t i = 0; i < size; i++) {
      size_t length = object[i].length();
      stream.write((const char *)&length, sizeof(length));
      stream.write((const char *)&object[i][0], length);
      result += sizeof(size_t) + length;
    }
    return result;
  }
  size_t Deserialize(std::vector<std::string> &object, std::istream &stream) {
    size_t result = 0;
    object.clear();
    size_t size;
    stream.read((char *)&size, sizeof(size_t));
    object.resize(size);
    result += size;
    for (size_t i = 0; i < size; i++) {
      size_t length;
      stream.read((char *)&length, sizeof(size_t));
      object[i].resize(length);
      stream.read((char *)&object[i][0], length);
      result += sizeof(size_t) + length;
    }
    return result;
  }
};

template <> class Serializer<JsonArray> {
public:
  size_t Serialize(JsonArray &array, std::ostream &stream) {
    return SerializeJsonArray(array, stream);
  }
  size_t Deserialize(JsonArray &array, std::istream &stream) {
    return DeserializeJsonArray(array, stream);
  }
};

template <> class Serializer<JsonObject> {
public:
  size_t Serialize(JsonObject &object, std::ostream &stream) {
    return SerializeJsonObject(object, stream);
  }
  size_t Deserialize(JsonObject &object, std::istream &stream) {
    return DeserializeJsonObject(object, stream);
  }
};

template <class K, class V> class Serializer<Map<K, V>> {
public:
  size_t Serialize(Map<K, V> &map, std::ostream &stream);
  size_t Deserialize(Map<K, V> &map, std::istream &stream);
};

template <class K, class V>
size_t Serializer<Map<K, V>>::Serialize(Map<K, V> &map, std::ostream &stream) {
  size_t bytes = 0;
  if (map.root_ == nullptr) {
    return bytes;
  }
  size_t size = map.Size();
  stream.write((const char *)&size, sizeof(size_t));
  bytes += sizeof(size_t);
  OuterNode<K, V> *cursor = map.FirstLeaf();
  Serializer<K> key_serializer;
  Serializer<V> value_serializer;
  size_t counter = 0;
  for (;;) {
    for (size_t i = 0; i < cursor->keys_.size(); i++) {
      bytes += key_serializer.Serialize(cursor->keys_[i], stream);
      bytes += value_serializer.Serialize(cursor->values_[i], stream);
      counter++;
    }
    if (cursor->next_) {
      cursor = cursor->next_;
      continue;
    }
    break;
  }
  if (counter != size) {
    throw std::runtime_error("unmatched tree size during serialization");
  }
  return bytes;
}

template <class K, class V>
size_t Serializer<Map<K, V>>::Deserialize(Map<K, V> &map,
                                          std::istream &stream) {
  auto find_fanout = [](size_t cache, size_t prio, size_t maximum) {
    if (cache >= 2 * prio) {
      return prio;
    } else {
      if (cache > maximum) {
        return cache / 2;
      } else {
        return cache;
      }
    }
  };
  size_t bytes = 0;
  map.Clear();
  size_t size;
  stream.read((char *)&size, sizeof(size_t));
  bytes += sizeof(size_t);
  map.size_ = size;
  const size_t outer_fanout = 3 * OUTER_FANOUT / 4;
  const size_t inner_fanout = 3 * INNER_FANOUT / 4;
  std::vector<Node *> cache;
  OuterNode<K, V> *outer = nullptr;
  OuterNode<K, V> *outer_previous = nullptr;
  InnerNode<K, V> *inner = nullptr;
  std::deque<std::pair<K, V>> kv_cache;
  std::pair<K, V> key_value_pair;
  size_t fanout;
  size_t pairs_read = 0;
  Serializer<K> key_serializer;
  Serializer<V> value_serializer;
  while (pairs_read < size || !kv_cache.empty()) {
    while (pairs_read < size && kv_cache.size() < 2 * outer_fanout) {
      bytes += key_serializer.Deserialize(key_value_pair.first, stream);
      bytes += value_serializer.Deserialize(key_value_pair.second, stream);
      kv_cache.emplace_back(key_value_pair);
      pairs_read++;
    }
    fanout = find_fanout(kv_cache.size(), outer_fanout, OUTER_FANOUT);
    outer = new OuterNode<K, V>();
    outer->keys_.resize(fanout);
    outer->values_.resize(fanout);
    for (size_t i = 0; i < fanout; i++) {
      outer->keys_[i] = kv_cache.front().first;
      outer->values_[i] = kv_cache.front().second;
      kv_cache.pop_front();
    }
    if (outer_previous) {
      outer_previous->next_ = outer;
      outer->previous_ = outer_previous;
      outer->next_ = nullptr;
    } else {
      outer->previous_ = nullptr;
    }
    outer_previous = outer;
    cache.push_back(outer);
  }
  for (;;) {
    size_t cache_size = cache.size();
    if (cache_size == 1) {
      map.root_ = cache[0];
      break;
    }
    size_t cache_index = 0;
    std::vector<Node *> cache_next;
    while (cache_size > 0) {
      fanout = find_fanout(cache_size, inner_fanout + 1, INNER_FANOUT + 1);
      cache_size -= fanout;
      inner = new InnerNode<K, V>();
      inner->keys_.resize(fanout - 1);
      inner->children_.resize(fanout);
      inner->children_[0] = cache[cache_index++];
      for (size_t i = 0; i < fanout - 1; i++) {
        if (cache[cache_index]->IsOuter()) {
          OuterNode<K, V> *node = (OuterNode<K, V> *)(cache[cache_index]);
          inner->keys_[i] = node->keys_.front();
        } else {
          InnerNode<K, V> *node = (InnerNode<K, V> *)(cache[cache_index]);
          inner->keys_[i] = node->keys_.front();
        }
        inner->children_[i + 1] = cache[cache_index++];
      }
      for (size_t i = 0; i < fanout; i++) {
        inner->children_[i]->SetParent(inner);
      }
      cache_next.push_back(inner);
    }
    cache = cache_next;
  }
  return bytes;
}

#endif