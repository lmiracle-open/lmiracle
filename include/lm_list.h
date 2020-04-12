/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_list.h
* Change Logs   :
* Date         Author      Notes
* 2019-11-06   linux       V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 系统文件
*******************************************************************************/

#ifndef __LM_LIST_H__
#define __LM_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * Simple doubly linked list implementation.
 */
struct lm_list_head {
	struct lm_list_head *next, *prev;
};

/**
 * Initialize the list as an empty list.
 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct lm_list_head name = LIST_HEAD_INIT(name)

static inline void lm_list_head_init(struct lm_list_head *list)
{
	list->next = list;
	list->prev = list;
}

/**
 * Insert a new entry between two known consecutive entries.
 */
static inline void __lm_list_add(	struct lm_list_head *entry,
									struct lm_list_head *prev,
									struct lm_list_head *next)
{
	next->prev = entry;
	entry->next = next;
	entry->prev = prev;
	prev->next = entry;
}

/**
 * Insert a new entry after the specified head.
 * @new: new entry to be added
 * @head: list head to add it after
 */
static inline void lm_list_add(	struct lm_list_head *entry,
								struct lm_list_head *head)
{
	__lm_list_add(entry, head, head->next);
}

/**
 * Insert a new entry before the specified head.
 * @new: new entry to be added
 * @head: list head to add it before
 */
static inline void lm_list_add_tail(struct lm_list_head *entry,
									struct lm_list_head *head)
{
	__lm_list_add(entry, head->prev, head);
}

/**
 * Delete a list entry by making the prev/next entries point to each other.
 */
static inline void __lm_list_del(	struct lm_list_head *prev,
									struct lm_list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline int lm_list_is_last(const struct lm_list_head *list,
                                  const struct lm_list_head *head)
{
    return list->next == head;
}

/**
 * deletes entry from list.
 * @entry: the element to delete from the list.
 */
static inline void lm_list_del(struct lm_list_head *entry)
{
	__lm_list_del(entry->prev, entry->next);
}

/**
 * deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void lm_list_del_init(struct lm_list_head *entry)
{
	__lm_list_del(entry->prev, entry->next);
	lm_list_head_init(entry);
}

/**
 * delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void lm_list_move_tail(struct lm_list_head *list,
				  struct lm_list_head *head)
{
	__lm_list_del(list->prev, list->next);
	lm_list_add_tail(list, head);
}

/**
 * tests whether a list is empty
 * @head: the list to test.
 */
static inline int lm_list_empty(const struct lm_list_head *head)
{
	return head->next == head;
}

/**
 * Returns a pointer to the container of this list element.
 * @param ptr Pointer to the struct list_head.
 * @param type Data type of the list element.
 * @param member Member name of the struct list_head field in the list element.
 * @return A pointer to the data struct containing the list head.
 */
#ifndef lm_container_of
#define lm_container_of(ptr, type, member) 							\
    (type *)((char *)(ptr) - (char *) &((type *)0)->member)
#endif

/**
 * get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define lm_list_entry(ptr, type, member) 								\
	lm_container_of(ptr, type, member)

/**
 * get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define lm_list_first_entry(ptr, type, member)							\
	lm_list_entry((ptr)->next, type, member)

/**
 * Retrieve the last list entry for the given listpointer.
 * @param ptr The list head
 * @param type Data type of the list element to retrieve
 * @param member Member name of the struct list_head field in the list element.
 * @return A pointer to the last list element.
 */
#define lm_list_last_entry(ptr, type, member)							\
    lm_list_entry((ptr)->prev, type, member)

#define __lm_container_of(ptr, sample, member)							\
    (void *)lm_container_of((ptr), __typeof__(*(sample)), member)

/**
 * iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.#include <stdbool.h>
 *
 * @member:	the name of the list_struct within the struct.
 */
#define lm_list_for_each_entry(pos, head, member)						\
	for (pos = lm_list_entry((head)->next, typeof(*pos), member);		\
	     &pos->member != (head);										\
	     pos = lm_list_entry(pos->member.next, typeof(*pos), member))

/**
 * iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define lm_list_for_each_entry_safe(pos, n, head, member)				\
	for (pos = lm_list_entry((head)->next, typeof(*pos), member),		\
		n = lm_list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head); 										\
	     pos = n, n = lm_list_entry(n->member.next, typeof(*n), member))

#ifdef __cplusplus
}
#endif

#endif  /* __LM_LIST_H__ */

/* end of file */

