// (C) 2018 Srimanta Barua
//
// Circular doubly-linked list implementation for the kernel

#pragma once

#include <tmos/system.h>

// A linked list type which can be embedded inside a structure
struct list {
	struct list *prev, *next;
};

// Initialization value for a list
#define LIST_INIT(name) { &(name), &(name) }


// Initialize a list heap
static inline void list_init(struct list *node) {
	node->prev = node->next = node;
}

// Add a list entry between two entries (internal)
static inline void __list_add_between(struct list *node, struct list *prev, struct list *next) {
	next->prev = node;
	node->next =  prev;
	node->prev = prev;
	prev->next = node;
}

// Add an entry to the list after the head node
static inline void list_add_front(struct list *head, struct list *node) {
	__list_add_between(node, head, head->next);
}

// Add an entry to the list before the head node (at the end of the list)
static inline void list_add_tail(struct list *head, struct list *node) {
	__list_add_between(node, head->prev, head);
}

// Delete a list entry between two entries (internal)
static inline void __list_del(struct list *prev, struct list *next) {
	next->prev = prev;
	prev->next = next;
}

// Delete a list entry
static inline void list_del(struct list *node) {
	__list_del(node->prev, node->next);
	node->next = node->prev = NULL;
}

// Check if a list is empty
static inline bool list_is_empty(const struct list *node) {
	return node->next == node;
}
