#pragma once

#include <string>
#include <vector>

/**
 * @brief 基于 26 个字母的字典树
 *
 */
class Trie {
 public:
  Trie() : childrens_(26) {
  }

 public:
  void Insert(const std::string& word) {
    Trie* node = this;
    for (const char ch : word) {
      size_t index = ch - 'a';
      if (node->childrens_[index] == nullptr) {
        node->childrens_[index] = new Trie();
      }
      node = node->childrens_[index];
    }
    node->is_end_ = true;
  }

 public:
  bool Search(const std::string& word) {
    Trie* node = this->SearchPrefix(word);
    return node != nullptr && node->is_end_;
  }

  bool StartWith(const std::string& prefix) {
    return this->SearchPrefix(prefix) != nullptr;
  }

 private:
  Trie* SearchPrefix(const std::string& prefix) {
    Trie* node = this;
    for (const char ch : prefix) {
      size_t index = ch - 'a';
      if (node->childrens_[index] == nullptr) {
        return nullptr;
      }
      node = node->childrens_[index];
    }
    return node;
  }

 private:
  std::vector<Trie*> childrens_;  // 所有子结点
  bool is_end_ = false;           // 表示该节点是否为字符串的结尾
};
