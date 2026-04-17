#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <iostream>
#include "disjoint_set.h"
#include <vector>
#include "data.h"
using namespace std;

template <typename T>
class Node {
public:
    T data;
    Node* next;
    Node* prev;

    Node(T value) : data(value), next(nullptr), prev(nullptr) {}
    ~Node() {
        // Case when handling 2-d linked list
        if constexpr (std::is_pointer_v<T>) {
            delete data; 
        }
    }
};

template <typename T>
class DoublyLinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
    vector<ClusterPoint>* points;
    int size;

public:
    DoublyLinkedList();
    ~DoublyLinkedList();

    Node<T>* sentinel;

    void append(T value);
    Node<T>* appendAndReturn(T value);
    void prepend(T value);
    void remove(Node<T>* node);
    int getSize() const;
};

struct GroupElement {
    int offset;
    int size;
};

typedef DoublyLinkedList<GroupElement> BucketGroup;
// typedef DoublyLinkedList<int> BucketGroup;

#endif // LINKED_LIST_H
