#ifndef __BS_LIST_H__
#define __BS_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

typedef struct __list_node {
    struct __list_node *prev, *next;
} list_node_t;

#define init_list_node(nd) do{(nd)->prev = (nd); (nd)->next = (nd); }while(0)

static inline void list_add_node_tail(list_node_t *__new, list_node_t *head)
{
    __new->prev = head->prev;
    __new->next = head;
    head->prev = __new;
    __new->prev->next = __new;
}

static inline void list_add_node(list_node_t *__new, list_node_t *head)
{
    __new->prev = head;
    __new->next = head->next;
    head->next = __new;
    __new->next->prev = __new;
}

static inline void list_del_node(list_node_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    init_list_node(node);
}

static inline int list_empty_node(list_node_t *node)
{
    return (((node)->prev == (node)) && ((node)->next == (node)));
}

static inline void list_replace(list_node_t *to, list_node_t *from)
{
    if (list_empty_node(from))
        return;

    to->next = from->next;
    to->prev = from->prev;
    from->next->prev = to;
    from->prev->next = to;
    init_list_node(from);
}

#define container_of(ptr, type, member) \
    (type *)((unsigned long)(ptr) - (unsigned long)&(((type *)0)->member))

#define list_each_node(head, node)               \
    for ((node) = (head)->next; (node) != (head); (node) = (node)->next)

#define list_each_node_safe(head, node, n)                                \
    for ((node) = (head)->next, (n) = (node)->next; (node) != (head); (node) = n, n = (n)->next)

#define list_each_node_reverse(head, node)               \
    for ((node) = (head)->prev; (node) != (head); (node) = (node)->prev)

#define list_each_entry(pos, head, member)             \
    for (pos = container_of((head)->next, typeof(*pos), member);    \
         &pos->member != (head);                                    \
         pos = container_of(pos->member.next, typeof(*pos), member))    \

#define list_each_entry_safe(pos, n, head, member)     \
    for (pos = container_of((head)->next, typeof(*pos), member),        \
             n = container_of(pos->member.next, typeof(*pos), member);  \
         &pos->member != (head);                                        \
         pos = n, n = container_of(n->member.next, typeof(*pos), member))

#endif //__BS_LIST_H__

