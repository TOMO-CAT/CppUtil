#pragma once

#include <atomic>

/*
 * Node1->next = Node2;
 * Node2->next = Node3;
 * 
 *              +------------------+ <-----+ stack top
 *              |                  |
 * head +---->  |      Node 1      |
 *              |                  |
 *              +------------------+
 *              |                  |
 *              |      Node 2      |
 *              |                  |
 *              +------------------+
 *              |                  |
 *              |      Node 3      |
 *              |                  |
 *              +------------------+ <-----+ stack bottom
 * 
 * 之所以没提供 Empty()、Size() 和 Top() 等方法, 是因为在并发场景下这些方法并没有意义
 */
template<typename T>
class LockFreeStack {
 public:
    struct Node {
        Node() : val(nullptr), next(nullptr) {}
        explicit Node(const T& v) : val(v), next(nullptr) {}
        T val;
        Node* next;
    };

 public:
    LockFreeStack() {
        head_.store(nullptr);
    }

    // 析构时弹出所有元素
    ~LockFreeStack() {
        while (true) {
            Node* node = Pop();
            if (node == nullptr) {
                break;
            }
            delete node;
        }
    }

    void Push(Node* new_node) {
        // 将栈顶指针指向新节点, CAS 直到成功
        while (true) {
            Node* original_head = head_.load();
            new_node->next = original_head;
            // compare_exchange_weak 允许两个数字进行原子交换: 第一个参数是期待的值, 第二个参数需要赋的新值
            // 1) 如果和期待值相同, 那么「赋值」成新值
            // 2) 如果和期待值不同, 那么将变量值和期待值「交换」
            //
            // compare_exchange_weak 性能比 compare_exchange_strong 更高, 但是可能在和期待值相同时返回 false, 这在一些循环算法中是可以接受的
            if (head_.compare_exchange_weak(original_head, new_node)) {
                return;
            }
        }
    }

    Node* Pop() {
        // 将栈顶指针指向下一节点, CAS 直到成功
        while (true) {
            Node* old_head = head_.load();
            if (!old_head) {
                return nullptr;
            }

            Node* new_head = old_head->next;
            if (head_.compare_exchange_weak(old_head, new_head)) {
                return old_head;
            }
        }
    }

 private:
    std::atomic<Node*> head_;  // 指向栈顶元素
};