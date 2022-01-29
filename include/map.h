/* Copyright 2022 Jonas Hegemann <jonas.hegemann@hotmail.de>

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

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <functional>
#include <map>
#include <shared_mutex>
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

#define CAST_INNER(node) static_cast<InnerNode<K, V> *>(node)
#define CAST_OUTER(node) static_cast<OuterNode<K, V> *>(node)
#define CAST_CONST_INNER(node) static_cast<const InnerNode<K, V> *>(node)
#define CAST_CONST_OUTER(node) static_cast<const OuterNode<K, V> *>(node)

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

template <class T> class Memory;
template <> class Memory<std::string>;
template <class K, class V> class Memory<std::map<K, V>>;
template <class K> class Memory<std::map<K, std::string>>;
template <class V> class Memory<std::map<std::string, V>>;
template <class T> class Memory<std::vector<T>>;
template <> class Memory<std::vector<std::string>>;
template <> class Memory<JsonObject>;
template <> class Memory<JsonArray>;
template <class K, class V> class Memory<Map<K, V>>;

class Node {
public:
  Node() {}
  virtual ~Node() {}
  virtual bool IsOuter() const = 0;
  virtual bool IsSparse() const = 0;
  virtual bool IsFull() const = 0;
  virtual bool IsEmpty() const = 0;
  virtual Node *GetParent() const = 0;
  virtual void SetParent(Node *node) = 0;
  virtual bool Redistribute(Node *node) = 0;
  virtual bool Coalesce(Node *node) = 0;
  virtual std::shared_mutex &SharedMutex() = 0;
};

template <class K, class V> class InnerNode : public Node {
  template <class> friend class ::Serializer;
  template <class> friend class ::Memory;
  template <class, class> friend class ::OuterNode;
  template <class, class> friend class ::Map;
  template <class, class> friend class ::MapIterator;
  template <class, class> friend class ::Multimap;
  template <class, class> friend class ::MultimapIterator;

public:
  InnerNode();
  virtual ~InnerNode();
  Node *GetParent() const;
  void SetParent(Node *node);
  bool IsOuter() const;
  bool IsSparse() const;
  bool IsFull() const;
  bool IsEmpty() const;
  size_t CountKeys() const;
  size_t CountChildren() const;
  K &Key(size_t index);
  const K &GetKey(size_t index) const;
  Node *Child(size_t index);
  const Node *GetChild(size_t index) const;
  size_t ChildIndex(const Node *child) const;
  size_t KeyIndex(const K &key) const;
  void Insert(Node *left, K &separator, Node *right);
  void Erase(const K &key, Node *child);
  std::pair<InnerNode<K, V> *, K> Split();
  size_t SeparatorIndex(InnerNode<K, V> *kin);
  bool Redistribute(Node *node);
  bool Coalesce(Node *node);
  std::shared_mutex &SharedMutex();

protected:
  Node *parent_;
  std::vector<K> keys_;
  std::vector<Node *> children_;
  std::shared_mutex mutex_;
};

template <class K, class V> InnerNode<K, V>::InnerNode() {
  keys_.reserve(INNER_FANOUT + 1);
  children_.reserve(INNER_FANOUT + 2);
}

template <class K, class V> InnerNode<K, V>::~InnerNode() {}

template <class K, class V> inline Node *InnerNode<K, V>::GetParent() const {
  return parent_;
}

template <class K, class V> inline void InnerNode<K, V>::SetParent(Node *node) {
  parent_ = node;
}

template <class K, class V> inline bool InnerNode<K, V>::IsOuter() const {
  return false;
}

template <class K, class V> inline bool InnerNode<K, V>::IsSparse() const {
  return keys_.size() < INNER_FANOUT / 2;
}

template <class K, class V> inline bool InnerNode<K, V>::IsFull() const {
  return keys_.size() > INNER_FANOUT;
}

template <class K, class V> inline bool InnerNode<K, V>::IsEmpty() const {
  return keys_.empty();
}

template <class K, class V> inline size_t InnerNode<K, V>::CountKeys() const {
  return keys_.size();
}

template <class K, class V>
inline size_t InnerNode<K, V>::CountChildren() const {
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
inline size_t InnerNode<K, V>::ChildIndex(const Node *child) const {
  if (children_.empty()) {
    return std::string::npos;
  }
  size_t position = 0;
  while (position < children_.size() && children_[position] != child) {
    position++;
  }
  if (position >= children_.size()) {
    return std::string::npos;
  }
  return position;
}

template <class K, class V>
inline size_t InnerNode<K, V>::KeyIndex(const K &key) const {
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
  size_t position = 0;
  while (position < keys_.size() && keys_[position] != key) {
    position++;
  }
  if (position >= keys_.size()) {
    return std::string::npos;
  }
  return position;
#endif
}

template <class K, class V>
void InnerNode<K, V>::Insert(Node *left, K &separator, Node *right) {
  if (keys_.empty()) {
    left->SetParent(this);
    right->SetParent(this);
    children_.push_back(left);
    children_.push_back(right);
    keys_.push_back(separator);
    return;
  }
  const size_t position = ChildIndex(left);
  if (position == std::string::npos) {
    throw std::runtime_error("tree: inner insert");
  }
  right->SetParent(this);
  keys_.insert(keys_.begin() + position, separator);
  children_.insert(children_.begin() + position + 1, right);
}

template <class K, class V>
void InnerNode<K, V>::Erase(const K &key, Node *child) {
  size_t key_position = KeyIndex(key);
  if (key_position == std::string::npos) {
    return;
  }
  size_t child_position = ChildIndex(child);
  if (child_position == std::string::npos) {
    return;
  }
  keys_.erase(keys_.begin() + key_position);
  children_.erase(children_.begin() + ChildIndex(child));
}

template <class K, class V>
std::pair<InnerNode<K, V> *, K> InnerNode<K, V>::Split() {
  const size_t size = keys_.size();
  const size_t keys_left = (size % 2 == 0) ? size / 2 : size / 2 + 1;
  const size_t children_left = keys_left + 1;
  InnerNode<K, V> *kin = new InnerNode<K, V>();
  const K up_key = keys_[keys_left];
  std::move(keys_.begin() + keys_left + 1, keys_.end(),
            std::back_inserter(kin->keys_));
  std::move(children_.begin() + children_left, children_.end(),
            std::back_inserter(kin->children_));
  keys_.erase(keys_.begin() + keys_left, keys_.end());
  children_.erase(children_.begin() + children_left, children_.end());
  for (auto it = kin->children_.begin(); it != kin->children_.end(); ++it) {
    (*it)->SetParent(kin);
  }
  kin->SetParent(parent_);
  return std::make_pair(kin, up_key);
}

template <class K, class V>
size_t InnerNode<K, V>::SeparatorIndex(InnerNode<K, V> *kin) {
  const size_t self_index = CAST_INNER(parent_)->ChildIndex(this);
  if (self_index == std::string::npos) {
    return std::string::npos;
  }
  const size_t kin_index = CAST_INNER(parent_)->ChildIndex(kin);
  if (kin_index == std::string::npos) {
    return std::string::npos;
  }
  if (abs((int)(self_index) - (int)(kin_index)) != 1) {
    return std::string::npos;
  }
  return std::min(self_index, kin_index);
}

template <class K, class V> bool InnerNode<K, V>::Redistribute(Node *node) {
  InnerNode<K, V> *kin = CAST_INNER(node);
  const size_t separator_index = SeparatorIndex(kin);
  if (separator_index == std::string::npos) {
    throw std::runtime_error("tree: inner redistribute");
  }
  K up_key = CAST_INNER(parent_)->keys_[separator_index];
  if (kin->keys_.size() >= keys_.size() + 2) {
    keys_.push_back(up_key);
    children_.push_back(kin->children_.front());
    kin->children_.erase(kin->children_.begin());
    children_.back()->SetParent(this);
    CAST_INNER(parent_)->keys_[separator_index] = kin->keys_[0];
    kin->keys_.erase(kin->keys_.begin());
    return true;
  }
  if (keys_.size() >= kin->keys_.size() + 2) {
    kin->keys_.insert(kin->keys_.begin(), up_key);
    kin->children_.insert(kin->children_.begin(), children_.back());
    children_.pop_back();
    kin->children_.front()->SetParent(kin);
    CAST_INNER(parent_)->keys_[separator_index] = keys_.back();
    keys_.pop_back();
    return true;
  }
  return false;
}

template <class K, class V> bool InnerNode<K, V>::Coalesce(Node *node) {
  InnerNode<K, V> *kin = CAST_INNER(node);
  if (keys_.size() + kin->keys_.size() > INNER_FANOUT) {
    return false;
  }
  const size_t separator_index = SeparatorIndex(kin);
  if (separator_index == std::string::npos) {
    throw std::runtime_error("tree: inner coalesce separator");
  }
  const K up_key = CAST_INNER(parent_)->keys_[separator_index];
  keys_.push_back(up_key);
  std::move(kin->keys_.begin(), kin->keys_.end(), std::back_inserter(keys_));
  kin->keys_.clear();
  for (auto it = kin->children_.begin(); it != kin->children_.end(); ++it) {
    (*it)->SetParent(this);
  }
  std::move(kin->children_.begin(), kin->children_.end(),
            std::back_inserter(children_));
  kin->children_.clear();
  return true;
}

template <class K, class V>
inline std::shared_mutex &InnerNode<K, V>::SharedMutex() {
  return mutex_;
}

template <class K, class V> class OuterNode : public Node {
  template <class> friend class ::Serializer;
  template <class> friend class ::Memory;
  template <class, class> friend class ::InnerNode;
  template <class, class> friend class ::Map;
  template <class, class> friend class ::MapIterator;
  template <class, class> friend class ::Multimap;
  template <class, class> friend class ::MultimapIterator;

public:
  OuterNode();
  virtual ~OuterNode();
  Node *GetParent() const;
  void SetParent(Node *node);
  bool IsOuter() const;
  bool IsSparse() const;
  bool IsFull() const;
  bool IsEmpty() const;
  size_t CountKeys() const;
  size_t CountValues() const;
  K &Key(size_t index);
  const K &GetKey(size_t index) const;
  V &Value(size_t index);
  const V &GetValue(size_t index) const;
  size_t ValueIndex(const V &value) const;
  size_t KeyIndex(const K &key) const;
  void Insert(const K &key, const V &value);
  void Erase(const K &key);
  MapIterator<K, V> Erase(const MapIterator<K, V> &iterator);
  std::pair<OuterNode<K, V> *, K> Split();
  bool Redistribute(Node *node);
  bool Coalesce(Node *node);
  OuterNode<K, V> *GetNext() const;
  OuterNode<K, V> *GetPrevious() const;
  std::shared_mutex &SharedMutex();

protected:
  std::vector<K> keys_;
  std::vector<V> values_;
  Node *parent_;
  OuterNode<K, V> *next_;
  OuterNode<K, V> *previous_;
  std::shared_mutex mutex_;
};

template <class K, class V>
OuterNode<K, V>::OuterNode() : next_(nullptr), previous_(nullptr) {
  keys_.reserve(OUTER_FANOUT + 1);
  values_.reserve(OUTER_FANOUT + 1);
}

template <class K, class V> OuterNode<K, V>::~OuterNode() {}

template <class K, class V> inline Node *OuterNode<K, V>::GetParent() const {
  return parent_;
}

template <class K, class V> inline void OuterNode<K, V>::SetParent(Node *node) {
  parent_ = node;
}

template <class K, class V> inline bool OuterNode<K, V>::IsOuter() const {
  return true;
}

template <class K, class V> inline bool OuterNode<K, V>::IsSparse() const {
  return keys_.size() < OUTER_FANOUT / 2;
}

template <class K, class V> inline bool OuterNode<K, V>::IsFull() const {
  return keys_.size() > OUTER_FANOUT;
}

template <class K, class V> inline bool OuterNode<K, V>::IsEmpty() const {
  return keys_.empty();
}

template <class K, class V> inline size_t OuterNode<K, V>::CountKeys() const {
  return keys_.size();
}

template <class K, class V> inline size_t OuterNode<K, V>::CountValues() const {
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

template <class K, class V>
size_t OuterNode<K, V>::ValueIndex(const V &value) const {
  if (values_.empty()) {
    return std::string::npos;
  }
  size_t position = 0;
  while (position < values_.size() && values_[position] != value) {
    position++;
  }
  if (position >= values_.size()) {
    return std::string::npos;
  }
  return position;
}

template <class K, class V>
size_t OuterNode<K, V>::KeyIndex(const K &key) const {
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
  size_t position = 0;
  while (position < keys_.size() && keys_[position] != key) {
    position++;
  }
  if (position >= keys_.size()) {
    return std::string::npos;
  }
  return position;
#endif
}

template <class K, class V>
void OuterNode<K, V>::Insert(const K &key, const V &value) {
  if (keys_.empty()) {
    keys_.push_back(key);
    values_.push_back(value);
    return;
  }
  size_t position = 0;
  while (position < keys_.size() && keys_[position] < key) {
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
  keys_.erase(keys_.begin() + key_position);
  values_.erase(values_.begin() + key_position);
}

template <class K, class V>
MapIterator<K, V> OuterNode<K, V>::Erase(const MapIterator<K, V> &iterator) {
  size_t index = iterator.index_;
  MapIterator<K, V> result = iterator;
  result++;
  keys_.erase(keys_.begin() + index);
  values_.erase(values_.begin() + index);
  return result;
}

template <class K, class V>
std::pair<OuterNode<K, V> *, K> OuterNode<K, V>::Split() {
  const size_t size = keys_.size();
  const size_t keys_left = (size % 2 == 0) ? size / 2 : size / 2 + 1;
  OuterNode<K, V> *kin = new OuterNode<K, V>();
  std::move(keys_.begin() + keys_left, keys_.end(),
            std::back_inserter(kin->keys_));
  std::move(values_.begin() + keys_left, values_.end(),
            std::back_inserter(kin->values_));
  keys_.erase(keys_.begin() + keys_left, keys_.end());
  values_.erase(values_.begin() + keys_left, values_.end());
  const K up_key = kin->keys_.front();
  kin->next_ = next_;
  kin->previous_ = this;
  if (next_ != nullptr) {
    next_->previous_ = kin;
  }
  next_ = kin;
  kin->parent_ = parent_;
  return std::make_pair(kin, up_key);
}

template <class K, class V> bool OuterNode<K, V>::Redistribute(Node *node) {
  OuterNode<K, V> *kin = CAST_OUTER(node);
  if (kin->keys_.size() >= keys_.size() + 2) {
    keys_.push_back(kin->keys_.front());
    values_.push_back(kin->values_.front());
    kin->keys_.erase(kin->keys_.begin());
    kin->values_.erase(kin->values_.begin());
  } else if (keys_.size() >= kin->keys_.size() + 2) {
    kin->keys_.insert(kin->keys_.begin(), keys_.back());
    kin->values_.insert(kin->values_.begin(), values_.back());
    keys_.pop_back();
    values_.pop_back();
  } else {
    return false;
  }
  const K up_key = kin->keys_.front();
  const size_t up_key_index = CAST_INNER(parent_)->ChildIndex(this);
  if (up_key_index == std::string::npos) {
    throw std::runtime_error("tree: outer redistribute");
  }
  CAST_INNER(parent_)->keys_[up_key_index] = up_key;
  return true;
}

template <class K, class V> bool OuterNode<K, V>::Coalesce(Node *node) {
  OuterNode<K, V> *kin = CAST_OUTER(node);
  if (kin->keys_.size() + keys_.size() > OUTER_FANOUT) {
    return false;
  }
  std::move(kin->keys_.begin(), kin->keys_.end(), std::back_inserter(keys_));
  std::move(kin->values_.begin(), kin->values_.end(),
            std::back_inserter(values_));
  kin->keys_.clear();
  kin->values_.clear();
  next_ = kin->next_;
  if (next_ != nullptr) {
    next_->previous_ = this;
  }
  return true;
}

template <class K, class V>
inline OuterNode<K, V> *OuterNode<K, V>::GetNext() const {
  return next_;
}

template <class K, class V>
inline OuterNode<K, V> *OuterNode<K, V>::GetPrevious() const {
  return previous_;
}

template <class K, class V>
inline std::shared_mutex &OuterNode<K, V>::SharedMutex() {
  return mutex_;
}

template <class K, class V> class Map {
  template <class> friend class ::Serializer;
  template <class> friend class ::Memory;
  template <class, class> friend class ::InnerNode;
  template <class, class> friend class ::OuterNode;
  template <class, class> friend class ::MapIterator;
  template <class, class> friend class ::Multimap;
  template <class, class> friend class ::MultimapIterator;

public:
  Map();
  virtual ~Map();
  void Clear();
  size_t Size() const;
  void Insert(const K &key, const V &value);
  void Update(const MapIterator<K, V> &iterator, const V &value);
  const V &operator[](const K &key) const;
  V &operator[](const K &key);
  bool Erase(const K &key);
  MapIterator<K, V> Erase(const MapIterator<K, V> &iterator);
  bool Contains(const K &key) const;
  MapIterator<K, V> Find(const K &key) const;
  MapIterator<K, V> Begin();
  const MapIterator<K, V> Begin() const;
  MapIterator<K, V> End();
  const MapIterator<K, V> End() const;

protected:
  Node *root_;
  size_t size_;
  const V &Get(const K &key) const;
  V &Get(const K &key);
  Node *LeftNode(Node *node) const;
  Node *RightNode(Node *node) const;
  size_t SeparatorIndex(Node *node, Node *kin) const;
  K SeparatorKey(Node *node, Node *kin) const;
  void PropagateUpwards(Node *origin, K &up_key, Node *kin);
  MapIterator<K, V> Locate(const K &key) const;
  OuterNode<K, V> *FirstLeaf() const;
  OuterNode<K, V> *LastLeaf() const;
  static size_t Fanout(size_t cache, size_t priority, size_t maximum);
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
        inner_node = CAST_INNER(current);
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

template <class K, class V> size_t Map<K, V>::Size() const { return size_; }

template <class K, class V> Node *Map<K, V>::LeftNode(Node *node) const {
  if (node == root_) {
    return nullptr;
  }
  InnerNode<K, V> *node_parent = CAST_INNER(node->GetParent());
  const size_t position = node_parent->ChildIndex(node);
  if (position == std::string::npos || position == 0) {
    return nullptr;
  }
  return node_parent->children_[position - 1];
}

template <class K, class V> Node *Map<K, V>::RightNode(Node *node) const {
  if (node == root_) {
    return nullptr;
  }
  InnerNode<K, V> *node_parent = CAST_INNER(node->GetParent());
  const size_t position = node_parent->ChildIndex(node);
  if (position == std::string::npos ||
      position == node_parent->children_.size() - 1) {
    return nullptr;
  }
  return node_parent->children_[position + 1];
}

template <class K, class V>
size_t Map<K, V>::SeparatorIndex(Node *node, Node *kin) const {
  InnerNode<K, V> *parent = CAST_INNER(node->GetParent());
  const size_t node_position = parent->ChildIndex(node);
  if (node_position == std::string::npos) {
    return std::string::npos;
  }
  const size_t kin_position = parent->ChildIndex(kin);
  if (kin_position == std::string::npos) {
    return std::string::npos;
  }
  if (abs((int)(node_position) - (int)(kin_position)) != 1) {
    throw std::runtime_error("tree: map separator index");
  }
  return std::min(node_position, kin_position);
}

template <class K, class V>
K Map<K, V>::SeparatorKey(Node *node, Node *kin) const {
  const size_t index = SeparatorIndex(node, kin);
  if (index == std::string::npos) {
    throw std::runtime_error("tree: map separator key");
  }
  InnerNode<K, V> *parent = CAST_INNER(node->GetParent());
  return parent->keys_[index];
}

template <class K, class V>
void Map<K, V>::PropagateUpwards(Node *origin, K &up_key, Node *kin) {
  if (origin == root_) {
    InnerNode<K, V> *inner_node = new InnerNode<K, V>();
    inner_node->Insert(origin, up_key, kin);
    root_ = inner_node;
    return;
  }
  InnerNode<K, V> *next_origin = CAST_INNER(origin->GetParent());
  next_origin->Insert(origin, up_key, kin);
  if (next_origin->IsFull()) {
    std::pair<Node *, K> extension = next_origin->Split();
    PropagateUpwards(next_origin, extension.second, extension.first);
  }
}

template <class K, class V>
MapIterator<K, V> Map<K, V>::Locate(const K &key) const {
  if (!root_) {
    return End();
  }
  Node *current = root_;
  while (!current->IsOuter()) {
    InnerNode<K, V> *inner_node = CAST_INNER(current);
    if (key < inner_node->keys_.front()) {
      current = inner_node->children_.front();
      continue;
    }
    if (key >= inner_node->keys_.back()) {
      current = inner_node->children_.back();
      continue;
    }
    for (size_t i = 0; i < inner_node->keys_.size() - 1; i++) {
      if (inner_node->keys_[i] <= key && key < inner_node->keys_[i + 1]) {
        current = inner_node->children_[i + 1];
        continue;
      }
    }
  }
  return MapIterator<K, V>(CAST_OUTER(current)->KeyIndex(key),
                           CAST_OUTER(current));
}

template <class K, class V> OuterNode<K, V> *Map<K, V>::FirstLeaf() const {
  if (root_ == nullptr) {
    return nullptr;
  }
  Node *current = root_;
  while (!current->IsOuter()) {
    current = CAST_INNER(current)->children_.front();
  }
  return CAST_OUTER(current);
}

template <class K, class V> OuterNode<K, V> *Map<K, V>::LastLeaf() const {
  if (root_ == nullptr) {
    return nullptr;
  }
  Node *current = root_;
  while (!current->IsOuter()) {
    current = CAST_INNER(current)->children_.back();
  }
  return CAST_OUTER(current);
}

template <class K, class V> const V &Map<K, V>::Get(const K &key) const {
  return Locate(key).GetValue();
}

template <class K, class V> V &Map<K, V>::Get(const K &key) {
  return Locate(key).GetValue();
}

template <class K, class V> const V &Map<K, V>::operator[](const K &key) const {
  return Get(key);
}

template <class K, class V> V &Map<K, V>::operator[](const K &key) {
  return Get(key);
}

template <class K, class V>
void Map<K, V>::Update(const MapIterator<K, V> &iterator, const V &value) {
  if (!iterator.node_ || (iterator.index_ == std::string::npos)) {
    return;
  }
  iterator.node_->values_[iterator.index_] = value;
}

template <class K, class V>
void Map<K, V>::Insert(const K &key, const V &value) {
  if (!root_) {
    root_ = new OuterNode<K, V>();
    CAST_OUTER(root_)->Insert(key, value);
    size_++;
    return;
  }
  MapIterator<K, V> iterator = Locate(key);
  if (iterator.node_ && iterator.index_ != std::string::npos) {
    Update(iterator, value);
    return;
  }
  iterator.node_->Insert(key, value);
  if (iterator.node_->IsFull()) {
    std::pair<Node *, K> extension = iterator.node_->Split();
    PropagateUpwards(iterator.node_, extension.second, extension.first);
  }
  size_++;
  return;
}

template <class K, class V>
MapIterator<K, V> Map<K, V>::Erase(const MapIterator<K, V> &iterator) {
  Node *current = iterator.node_;
  MapIterator<K, V> result = CAST_OUTER(current)->Erase(iterator);
  size_--;
  if (current == root_ && root_->IsOuter() && root_->IsEmpty()) {
    delete root_;
    root_ = nullptr;
    return result;
  }
  Node *left;
  Node *right;
  size_t current_size = -1;
  while (current != root_) {
    if (!current->IsSparse()) {
      return result;
    }
    left = LeftNode(current);
    if (left != nullptr && left->Redistribute(current)) {
      if (current->IsOuter() && result.node_ == current) {
        result.index_++;
      }
      return result;
    }
    right = RightNode(current);
    if (right != nullptr && current->Redistribute(right)) {
      if (current->IsOuter() && result.node_ == right) {
        result.node_ = CAST_OUTER(current);
        result.index_ = CAST_OUTER(current)->CountKeys() - 1;
      }
      return result;
    }
    if (current->IsOuter()) {
      current_size = CAST_OUTER(current)->CountKeys();
    }
    if (left != nullptr && left->Coalesce(current)) {
      if (current->IsOuter() && result.node_ == current) {
        result.node_ = CAST_OUTER(left);
        result.index_ += CAST_OUTER(left)->CountKeys() - current_size;
      }
      InnerNode<K, V> *parent = CAST_INNER(current->GetParent());
      const K separator_key = SeparatorKey(left, current);
      parent->Erase(separator_key, current);
      delete current;
      current = left->GetParent();
      continue;
    }
    if (right != nullptr && current->Coalesce(right)) {
      if (current->IsOuter() && result.node_ == right) {
        result.node_ = CAST_OUTER(current);
        result.index_ = current_size;
      }
      InnerNode<K, V> *parent = CAST_INNER(current->GetParent());
      const K separator_key = SeparatorKey(current, right);
      parent->Erase(separator_key, right);
      delete right;
      current = current->GetParent();
      continue;
    }
  }
  InnerNode<K, V> *inner_node = CAST_INNER(current);
  if (inner_node->keys_.empty()) {
    Node *backup = root_;
    root_ = inner_node->children_.front();
    root_->SetParent(nullptr);
    delete backup;
  }
  return result;
}

template <class K, class V> bool Map<K, V>::Erase(const K &key) {
  MapIterator<K, V> iterator = Locate(key);
  if (iterator.index_ == std::string::npos) {
    return false;
  }
  Erase(iterator);
  return true;
}

template <class K, class V> bool Map<K, V>::Contains(const K &key) const {
  MapIterator<K, V> iterator = Locate(key);
  if (iterator.index_ == std::string::npos) {
    return false;
  }
  return true;
}

template <class K, class V>
MapIterator<K, V> Map<K, V>::Find(const K &key) const {
  MapIterator<K, V> iterator = Locate(key);
  if (iterator.index_ == std::string::npos) {
    iterator.node_ = nullptr;
  }
  return iterator;
}

template <class K, class V> MapIterator<K, V> Map<K, V>::Begin() {
  if (root_ == nullptr) {
    return End();
  }
  MapIterator<K, V> iter;
  iter.node_ = FirstLeaf();
  iter.index_ = 0;
  return iter;
}

template <class K, class V> const MapIterator<K, V> Map<K, V>::Begin() const {
  if (root_ == nullptr) {
    return End();
  }
  MapIterator<K, V> iter;
  iter.node_ = FirstLeaf();
  iter.index_ = 0;
  return iter;
}

template <class K, class V> MapIterator<K, V> Map<K, V>::End() {
  return MapIterator<K, V>();
}

template <class K, class V> const MapIterator<K, V> Map<K, V>::End() const {
  return MapIterator<K, V>();
}

template <class K, class V>
size_t Map<K, V>::Fanout(size_t cache, size_t priority, size_t maximum) {
  if (cache >= 2 * priority) {
    return priority;
  } else {
    if (cache > maximum) {
      return cache / 2;
    } else {
      return cache;
    }
  }
}

template <class K, class V> class MapIterator {
  template <class> friend class ::Serializer;
  template <class> friend class ::Memory;
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
  MapIterator(size_t index, OuterNode<K, V> *node);
  const size_t GetIndex() const;
  const OuterNode<K, V> *GetNode() const;
  K &Key();
  V &Value();
  OuterNode<K, V> *node_;
  size_t index_;
  void Increment();
  void Decrement();
};

template <class K, class V>
MapIterator<K, V>::MapIterator() : node_(nullptr), index_(std::string::npos) {}

template <class K, class V>
MapIterator<K, V>::MapIterator(size_t index, OuterNode<K, V> *node)
    : node_(node), index_(index) {}

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

template <class K, class V>
inline const size_t MapIterator<K, V>::GetIndex() const {
  return index_;
}

template <class K, class V>
inline const OuterNode<K, V> *MapIterator<K, V>::GetNode() const {
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
  template <class> friend class ::Memory;
  template <class, class> friend class ::InnerNode;
  template <class, class> friend class ::OuterNode;
  template <class, class> friend class ::Map;
  template <class, class> friend class ::MapIterator;
  template <class, class> friend class ::MultimapIterator;

public:
  Multimap();
  virtual ~Multimap();
  size_t Size() const;
  void Insert(const K &key, const V &value);
  void Insert(MultimapIterator<K, V> &iter, const V &value);
  const std::vector<V> &operator[](const K &key) const;
  std::vector<V> &operator[](const K &key);
  void Clear();
  bool Erase(const K &key);
  bool Erase(const K &key, const V &value);
  bool Erase(MultimapIterator<K, V> &iter);
  bool Contains(const K &key) const;
  MultimapIterator<K, V> Find(const K &key) const;
  MultimapIterator<K, V> Begin();
  const MultimapIterator<K, V> Begin() const;
  MultimapIterator<K, V> End();
  const MultimapIterator<K, V> End() const;

protected:
  Map<K, std::vector<V>> tree_;
  const std::vector<V> &Get(const K &key) const;
  std::vector<V> &Get(const K &key);
  MultimapIterator<K, V> BeginIterator();
};

template <class K, class V> Multimap<K, V>::Multimap() {}

template <class K, class V> Multimap<K, V>::~Multimap() {}

template <class K, class V> size_t Multimap<K, V>::Size() const {
  return tree_.Size();
}

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
inline const std::vector<V> &Multimap<K, V>::Get(const K &key) const {
  return tree_.Get(key);
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
inline bool Multimap<K, V>::Erase(MultimapIterator<K, V> &iter) {
  return Erase(iter.GetKey(), iter.GetValue());
}

template <class K, class V>
inline bool Multimap<K, V>::Contains(const K &key) const {
  return tree_.Contains(key);
}

template <class K, class V>
MultimapIterator<K, V> Multimap<K, V>::Find(const K &key) const {
  MapIterator<K, std::vector<V>> iter = tree_.Find(key);
  MultimapIterator<K, V> multi_iter;
  if (iter != tree_.End()) {
    multi_iter.index_ = iter.index_;
    multi_iter.multi_index_ = 0;
    multi_iter.node_ = iter.node_;
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
  template <class> friend class ::Memory;
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
  static size_t Serialize(const T &object, std::ostream &stream,
                          const std::atomic<bool> &cancel = false) {
    stream.write((const char *)&object, sizeof(T));
    return stream ? sizeof(T) : std::string::npos;
  }
  static size_t Deserialize(T &object, std::istream &stream,
                            const std::atomic<bool> &cancel = false) {
    stream.read((char *)&object, sizeof(T));
    return stream ? sizeof(T) : std::string::npos;
  }
};

template <> class Serializer<std::string> {
public:
  static size_t Serialize(const std::string &object, std::ostream &stream,
                          const std::atomic<bool> &cancel = false) {
    size_t length = object.length();
    stream.write((const char *)&length, sizeof(size_t));
    stream.write((const char *)&object[0], length);
    return stream ? sizeof(size_t) + length : std::string::npos;
  }
  static size_t Deserialize(std::string &object, std::istream &stream,
                            const std::atomic<bool> &cancel = false) {
    object.clear();
    size_t length;
    stream.read((char *)&length, sizeof(size_t));
    object.resize(length);
    stream.read((char *)&object[0], length);
    return stream ? sizeof(size_t) + length : std::string::npos;
  }
};

template <class K, class V> class Serializer<std::map<K, V>> {
public:
  static size_t Serialize(const std::map<K, V> &object, std::ostream &stream,
                          const std::atomic<bool> &cancel = false) {
    typename std::map<K, V>::iterator it;
    size_t size = object.size();
    stream.write((const char *)&size, sizeof(size_t));
    for (it = object.begin(); it != object.end(); ++it) {
      if (cancel) {
        return std::string::npos;
      }
      stream.write((const char *)&it->first, sizeof(K));
      stream.write((const char *)&it->second, sizeof(V));
    }
    return stream ? sizeof(size_t) + size * (sizeof(K) + sizeof(V))
                  : std::string::npos;
  }
  static size_t Deserialize(std::map<K, V> &object, std::istream &stream,
                            const std::atomic<bool> &cancel = false) {
    object.clear();
    size_t size;
    stream.read((char *)&size, sizeof(size_t));
    K key;
    V value;
    for (size_t i = 0; i < size; i++) {
      if (cancel) {
        return std::string::npos;
      }
      stream.read((char *)&key, sizeof(K));
      stream.read((char *)&value, sizeof(V));
      object.insert(key, value);
    }
    return stream ? sizeof(size_t) + size * (sizeof(K) + sizeof(V))
                  : std::string::npos;
  }
};

template <class K> class Serializer<std::map<K, std::string>> {
public:
  static size_t Serialize(const std::map<K, std::string> &object,
                          std::ostream &stream,
                          const std::atomic<bool> &cancel = false) {
    size_t result = 0;
    size_t size = object.size();
    stream.write((const char *)&size, sizeof(size_t));
    result += sizeof(size_t);
    for (auto it = object.begin(); it != object.end(); ++it) {
      if (cancel) {
        return std::string::npos;
      }
      stream.write((const char *)&it->first, sizeof(K));
      size_t length = it->second.length();
      stream.write((const char *)&it->second[0], length);
      result += sizeof(size_t) + length;
    }
    result += size * sizeof(K);
    return stream ? result : std::string::npos;
  }
  static size_t Deserialize(std::map<K, std::string> &object,
                            std::istream &stream,
                            const std::atomic<bool> &cancel = false) {
    object.clear();
    size_t result = 0;
    size_t size;
    stream.read((char *)&size, sizeof(size_t));
    K key;
    std::string value;
    for (size_t i = 0; i < size; i++) {
      if (cancel) {
        return std::string::npos;
      }
      stream.read((char *)&key, sizeof(K));
      size_t length;
      stream.read((char *)&length, sizeof(size_t));
      value.resize(length);
      stream.read((char *)&value[0], length);
      result += sizeof(size_t) + length;
    }
    result += size * sizeof(K);
    return stream ? result : std::string::npos;
  }
};

template <class V> class Serializer<std::map<std::string, V>> {
public:
  static size_t Serialize(const std::map<std::string, V> &object,
                          std::ostream &stream,
                          const std::atomic<bool> &cancel = false) {
    size_t result = 0;
    size_t size = object.size();
    stream.write((const char *)&size, sizeof(size_t));
    result += sizeof(size_t);
    for (auto it = object.begin(); it != object.end(); ++it) {
      if (cancel) {
        return std::string::npos;
      }
      size_t length = it->first.length();
      stream.write((const char *)&it->first[0], length);
      result += sizeof(size_t) + length;
      stream.write((const char *)&it->second, sizeof(V));
    }
    result += size * sizeof(V);
    return stream ? result : std::string::npos;
  }
  static size_t Deserialize(std::map<std::string, V> &object,
                            std::istream &stream,
                            const std::atomic<bool> &cancel = false) {
    object.clear();
    size_t result = 0;
    size_t size;
    stream.read((char *)&size, sizeof(size_t));
    std::string key;
    V value;
    for (size_t i = 0; i < size; i++) {
      if (cancel) {
        return std::string::npos;
      }
      size_t length;
      stream.read((char *)&length, sizeof(size_t));
      key.resize(length);
      stream.read((char *)&key[0], length);
      result += sizeof(size_t) + length;
      stream.read((char *)&value, sizeof(V));
    }
    result += size * sizeof(V);
    return stream ? result : std::string::npos;
  }
};

template <class T> class Serializer<std::vector<T>> {
public:
  static size_t Serialize(const std::vector<T> &object, std::ostream &stream,
                          const std::atomic<bool> &cancel = false) {
    size_t size = object.size();
    stream.write((const char *)&size, sizeof(size_t));
    for (size_t i = 0; i < size; i++) {
      if (cancel) {
        return std::string::npos;
      }
      stream.write((const char *)&object[i], sizeof(T));
    }
    return stream ? sizeof(size_t) + size * sizeof(T) : std::string::npos;
  }
  static size_t Deserialize(std::vector<T> &object, std::istream &stream,
                            const std::atomic<bool> &cancel = false) {
    object.clear();
    size_t size;
    stream.read((char *)&size, sizeof(size_t));
    object.resize(size);
    for (size_t i = 0; i < size; i++) {
      if (cancel) {
        return std::string::npos;
      }
      stream.read((char *)&object[i], sizeof(T));
    }
    return stream ? sizeof(size_t) + size * sizeof(T) : std::string::npos;
  }
};

template <> class Serializer<std::vector<std::string>> {
public:
  static size_t Serialize(const std::vector<std::string> &object,
                          std::ostream &stream,
                          const std::atomic<bool> &cancel = false) {
    size_t result = 0;
    size_t size = object.size();
    stream.write((const char *)&size, sizeof(size_t));
    result += sizeof(size_t);
    for (size_t i = 0; i < size; i++) {
      if (cancel) {
        return std::string::npos;
      }
      size_t length = object[i].length();
      stream.write((const char *)&length, sizeof(length));
      stream.write((const char *)&object[i][0], length);
      result += sizeof(size_t) + length;
    }
    return stream ? result : std::string::npos;
  }
  static size_t Deserialize(std::vector<std::string> &object,
                            std::istream &stream,
                            const std::atomic<bool> &cancel = false) {
    size_t result = 0;
    object.clear();
    size_t size;
    stream.read((char *)&size, sizeof(size_t));
    object.resize(size);
    result += size;
    for (size_t i = 0; i < size; i++) {
      if (cancel) {
        return std::string::npos;
      }
      size_t length;
      stream.read((char *)&length, sizeof(size_t));
      object[i].resize(length);
      stream.read((char *)&object[i][0], length);
      result += sizeof(size_t) + length;
    }
    return stream ? result : std::string::npos;
  }
};

template <> class Serializer<JsonArray> {
public:
  static size_t Serialize(const JsonArray &object, std::ostream &stream,
                          const std::atomic<bool> &cancel = false) {
    return json::Serialize(object, stream);
  }
  static size_t Deserialize(JsonArray &object, std::istream &stream,
                            const std::atomic<bool> &cancel = false) {
    return json::Deserialize(object, stream);
  }
};

template <> class Serializer<JsonObject> {
public:
  static size_t Serialize(const JsonObject &object, std::ostream &stream,
                          const std::atomic<bool> &cancel = false) {
    return json::Serialize(object, stream);
  }
  static size_t Deserialize(JsonObject &object, std::istream &stream,
                            const std::atomic<bool> &cancel = false) {
    return json::Deserialize(object, stream);
  }
};

template <class K, class V> class Serializer<Map<K, V>> {
public:
  static size_t Serialize(const Map<K, V> &object, std::ostream &stream,
                          const std::atomic<bool> &cancel = false);
  static size_t Deserialize(Map<K, V> &object, std::istream &stream,
                            const std::atomic<bool> &cancel = false);
};

template <class K, class V>
size_t Serializer<Map<K, V>>::Serialize(const Map<K, V> &object,
                                        std::ostream &stream,
                                        const std::atomic<bool> &cancel) {
  size_t bytes = 0;
  if (object.root_ == nullptr) {
    return bytes;
  }
  size_t size = object.Size();
  stream.write((const char *)&size, sizeof(size_t));
  bytes += sizeof(size_t);
  OuterNode<K, V> *cursor = object.FirstLeaf();
  size_t counter = 0;
  for (;;) {
    if (cancel) {
      return std::string::npos;
    }
    for (size_t i = 0; i < cursor->keys_.size(); i++) {
      bytes += Serializer<K>::Serialize(cursor->keys_[i], stream);
      bytes += Serializer<V>::Serialize(cursor->values_[i], stream);
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
  return stream ? bytes : std::string::npos;
}

template <class K, class V>
size_t Serializer<Map<K, V>>::Deserialize(Map<K, V> &object,
                                          std::istream &stream,
                                          const std::atomic<bool> &cancel) {
  size_t bytes = 0;
  object.Clear();
  size_t size;
  stream.read((char *)&size, sizeof(size_t));
  bytes += sizeof(size_t);
  object.size_ = size;
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
  while (pairs_read < size || !kv_cache.empty()) {
    if (cancel) {
      return std::string::npos;
    }
    while (pairs_read < size && kv_cache.size() < 2 * outer_fanout) {
      bytes += Serializer<K>::Deserialize(key_value_pair.first, stream);
      bytes += Serializer<V>::Deserialize(key_value_pair.second, stream);
      kv_cache.emplace_back(key_value_pair);
      pairs_read++;
    }
    fanout = Map<K, V>::Fanout(kv_cache.size(), outer_fanout, OUTER_FANOUT);
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
    if (cancel) {
      return std::string::npos;
    }
    size_t cache_size = cache.size();
    if (cache_size == 1) {
      object.root_ = cache[0];
      break;
    }
    size_t cache_index = 0;
    std::vector<Node *> cache_next;
    while (cache_size > 0) {
      fanout =
          Map<K, V>::Fanout(cache_size, inner_fanout + 1, INNER_FANOUT + 1);
      cache_size -= fanout;
      inner = new InnerNode<K, V>();
      inner->keys_.resize(fanout - 1);
      inner->children_.resize(fanout);
      inner->children_[0] = cache[cache_index++];
      for (size_t i = 0; i < fanout - 1; i++) {
        if (cache[cache_index]->IsOuter()) {
          OuterNode<K, V> *node = CAST_OUTER(cache[cache_index]);
          inner->keys_[i] = node->keys_.front();
        } else {
          InnerNode<K, V> *node = CAST_INNER(cache[cache_index]);
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
  return stream ? bytes : std::string::npos;
}

template <class T> class Memory {
public:
  static uint64_t Consumption(const T &object) { return sizeof(object); }
};

template <> class Memory<std::string> {
public:
  static uint64_t Consumption(const std::string &object) {
    return sizeof(std::string) + object.capacity();
  }
};

template <class K, class V> class Memory<std::map<K, V>> {
public:
  static uint64_t Consumption(const std::map<K, V> &object) {
    uint64_t result = sizeof(std::map<K, V>);
    for (auto it = object.begin(); it != object.end(); it++) {
      result += sizeof(K) + sizeof(V);
    }
    return result;
  }
};

template <class K> class Memory<std::map<K, std::string>> {
public:
  static uint64_t Consumption(const std::map<K, std::string> &object) {
    uint64_t result = sizeof(std::map<K, std::string>);
    for (auto it = object.begin(); it != object.end(); it++) {
      result += sizeof(K) + Memory<std::string>::Consumption(object->second);
    }
    return result;
  }
};

template <class V> class Memory<std::map<std::string, V>> {
public:
  static uint64_t Consumption(const std::map<std::string, V> &object) {
    uint64_t result = sizeof(std::map<std::string, V>);
    for (auto it = object.begin(); it != object.end(); it++) {
      result += Memory<std::string>::Consumption(object->first) + sizeof(V);
    }
    return result;
  }
};

template <class T> class Memory<std::vector<T>> {
public:
  static uint64_t Consumption(const std::vector<T> &object) {
    return sizeof(T) * object.capacity();
  }
};

template <> class Memory<std::vector<std::string>> {
public:
  static uint64_t Consumption(const std::vector<std::string> &object) {
    uint64_t result = sizeof(std::vector<std::string>);
    for (size_t i = 0; i < object.size(); i++) {
      result += Memory<std::string>::Consumption(object[i]);
    }
    return result;
  }
};

template <> class Memory<JsonObject> {
public:
  static uint64_t Consumption(const JsonObject &object) {
    return json::Memory(object);
  }
};

template <> class Memory<JsonArray> {
public:
  static uint64_t Consumption(const JsonArray &object) {
    return json::Memory(object);
  }
};

template <class K, class V> class Memory<Map<K, V>> {
public:
  static uint64_t Consumption(const Map<K, V> &object) {
    uint64_t result = sizeof(Map<K, V>);
    if (object.root_ == nullptr) {
      return result;
    }
    std::stack<const Node *> todo;
    todo.push(object.root_);
    while (!todo.empty()) {
      const Node *current = todo.top();
      todo.pop();
      if (!current->IsOuter()) {
        const InnerNode<K, V> *inner = CAST_CONST_INNER(current);
        result += sizeof(InnerNode<K, V>);
        for (size_t i = 0; i < inner->CountKeys(); i++) {
          result += Memory<K>::Consumption(inner->GetKey(i));
        }
        result += (inner->keys_.capacity() - inner->CountKeys()) * sizeof(K);
        result += sizeof(void *) * inner->children_.capacity();
        for (size_t i = 0; i < inner->CountChildren(); i++) {
          todo.push(inner->GetChild(i));
        }
      } else {
        const OuterNode<K, V> *outer = CAST_CONST_OUTER(current);
        result += sizeof(OuterNode<K, V>);
        for (size_t i = 0; i < outer->CountKeys(); i++) {
          result += Memory<K>::Consumption(outer->GetKey(i));
        }
        result += (outer->keys_.capacity() - outer->CountKeys()) * sizeof(K);
        for (size_t i = 0; i < outer->CountValues(); i++) {
          result += Memory<V>::Consumption(outer->GetValue(i));
        }
        result +=
            (outer->values_.capacity() - outer->CountValues()) * sizeof(V);
      }
    }
    return result;
  }
};

#endif