#include "linked_list.h"
template <typename T>
DoublyLinkedList<T>::DoublyLinkedList() {
    sentinel = new Node<T>(T()); // Sentinel node with default value
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    size = 0;
}

template <typename T>
DoublyLinkedList<T>::~DoublyLinkedList() {
    Node<T>* current = sentinel->next;
    while (current != sentinel) {
        Node<T>* next = current->next;
        delete current;
        current = next;
    }
    delete sentinel;
}

template <typename T>
void DoublyLinkedList<T>::append(T value) {
    Node<T>* newNode = new Node<T>(value);
    newNode->prev = sentinel->prev;
    newNode->next = sentinel;
    sentinel->prev->next = newNode;
    sentinel->prev = newNode;
    size += 1;
}

template <typename T>
Node<T>* DoublyLinkedList<T>::appendAndReturn(T value) {
    Node<T>* newNode = new Node<T>(value);
    newNode->prev = sentinel->prev;
    newNode->next = sentinel;
    sentinel->prev->next = newNode;
    sentinel->prev = newNode;
    size += 1;
    return newNode;
}

template <typename T>
void DoublyLinkedList<T>::prepend(T value) {
    Node<T>* newNode = new Node<T>(value);
    newNode->next = sentinel->next;
    newNode->prev = sentinel;
    sentinel->next->prev = newNode;
    sentinel->next = newNode;
    size += 1;
}

template <typename T>
void DoublyLinkedList<T>::remove(Node<T>* node) {
    if (node == sentinel) return; // Do not remove the sentinel node
    node->prev->next = node->next;
    node->next->prev = node->prev;
    delete node;
    size -= 1;
}

template <typename T>
int DoublyLinkedList<T>::getSize() const {
    return size;
}

template class DoublyLinkedList<int>;
template class DoublyLinkedList<long>;
template class DoublyLinkedList<vector<int>>;
template class DoublyLinkedList<BucketGroup*>;
template class DoublyLinkedList<GroupElement>;
