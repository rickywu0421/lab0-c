#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head;

    head = (struct list_head *) malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    struct list_head *pos, *safe;

    if (!l)
        return;

    list_for_each_safe (pos, safe, l) {
        element_t *ele;

        ele = list_entry(pos, element_t, list);
        free(ele->value);
        free(ele);
    }

    free(l);
}

/**
 * Implement the common operations of q_insert_head and q_insert_tail,
 * which is allocate heap space for queue element and copy string into
 * it, after this function return, the @ele is ready for later use.
 */
static bool __q_insert(element_t **ele, char *s)
{
    size_t len;

    if (!s)
        return false;

    *ele = (element_t *) malloc(sizeof(element_t));
    if (!(*ele))
        return false;

    len = strlen(s);
    (*ele)->value = (char *) malloc(len + 1);
    if (!(*ele)->value) {
        free(*ele);
        return false;
    }

    /* Copy string into queue element */
    strncpy((*ele)->value, s, len + 1);

    return true;
}
/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *ele;

    if (!head)
        return false;

    /* Insert queue element into head of @head */
    if (__q_insert(&ele, s)) {
        list_add(&ele->list, head);

        return true;
    }

    return false;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *ele;

    if (!head)
        return false;

    /* Insert queue element into tail of @head */
    if (__q_insert(&ele, s)) {
        list_add_tail(&ele->list, head);

        return true;
    }

    return false;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    element_t *first;

    if (!head || list_empty(head))
        return NULL;

    first = list_entry(head->next, element_t, list);
    list_del(&first->list);

    if (sp && bufsize) {
        strncpy(sp, first->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return first;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    element_t *last;

    if (!head || list_empty(head))
        return NULL;

    last = list_entry(head->prev, element_t, list);
    list_del(&last->list);

    if (sp && bufsize) {
        strncpy(sp, last->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return last;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    struct list_head *pos;
    int size;

    if (!head)
        return 0;

    size = 0;
    list_for_each (pos, head)
        size++;

    return size;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    element_t *ele;
    struct list_head *slow, *fast;

    if (!head || list_empty(head))
        return false;

    for (slow = head->next, fast = head->next;
         fast != head && fast->next != head;
         slow = slow->next, fast = fast->next->next)
        ;

    list_del(slow);

    ele = list_entry(slow, element_t, list);
    free(ele->value);
    free(ele);

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    element_t *pos, *next;
    int dup = 0;

    if (!head)
        return false;

    list_for_each_entry_safe (pos, next, head, list) {
        if (&next->list != head && !strcmp(pos->value, next->value)) {
            list_del(&pos->list);
            free(pos->value);
            free(pos);
            dup = 1;
        } else if (dup) {
            list_del(&pos->list);
            free(pos->value);
            free(pos);
            dup = 0;
        }
    }

    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    struct list_head *pos;

    if (!head || list_empty(head) || list_is_singular(head))
        return;

    list_for_each (pos, head) {
        if (pos->next == head)
            break;

        list_del(pos);
        list_add(pos, pos->next);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    struct list_head *pos, *next;

    if (!head || list_empty(head) || list_is_singular(head))
        return;

    /* Start from the real head, not the first queue element */
    pos = head, next = head->next;
    do {
        pos->next = pos->prev;
        pos->prev = next;
        pos = next;
        next = next->next;
    } while (pos != head);
}

/* Split the list into two sublists, and return the middle node of
   the list */
static struct list_head *__q_split_into_two(struct list_head *head)
{
    struct list_head *fast, *slow;

    /* Find the middle node of the list */
    for (fast = head, slow = head; fast && fast->next;
         fast = fast->next->next, slow = slow->next)
        ;

    slow->prev->next = NULL;

    return slow;
}

/* Merge two sorted list, and return the head of sorted list */
static struct list_head *__q_merge_two_lists(struct list_head *left,
                                             struct list_head *right)
{
    struct list_head *head, **ptr;

    head = NULL;
    ptr = &head;

    for (; left && right; ptr = &(*ptr)->next) {
        element_t *e_left, *e_right;

        e_left = list_entry(left, element_t, list);
        e_right = list_entry(right, element_t, list);

        if (strcmp(e_left->value, e_right->value) <= 0) {
            *ptr = left;
            left = left->next;
        } else {
            *ptr = right;
            right = right->next;
        }
    }

    *ptr = (struct list_head *) ((uintptr_t) left | (uintptr_t) right);

    return head;
}

/* Recursive merge sort function, return the sorted sublist
   which is singly linked list */
static struct list_head *__q_sort(struct list_head *left)
{
    if (left->next) {
        struct list_head *right;

        right = __q_split_into_two(left);

        left = __q_sort(left);
        right = __q_sort(right);

        return __q_merge_two_lists(left, right);
    }
    return left;
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    struct list_head *list, *pos, *prev;

    if (!head || list_empty(head) || list_is_singular(head))
        return;

    /* Make the last node's next to NULL, avoiding the result of
       __q_split_into_two() is wrong */
    head->prev->next = NULL;

    list = head->next;
    list = __q_sort(list);

    /* Now the list is a sorted singly-linked list,
       it's time to recover the list into doubly-linked list */
    head->next = list;

    for (prev = head, pos = head->next; pos; prev = pos, pos = pos->next)
        pos->prev = prev;

    prev->next = head;
    head->prev = prev;
}
