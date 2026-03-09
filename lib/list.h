#ifndef _LIST_H_
#define _LIST_H_

/*
 * list.h — Lista enlazada circular doblemente enlazada (estilo Linux/FreeBSD)
 *
 * Diseño "intrusivo": el nodo de lista (list_head) se EMBEBE dentro del
 * struct de datos. Para recuperar el struct padre desde un list_head* se
 * usa container_of().
 *
 * Estructura de una lista con 2 elementos (A y B):
 *
 *   head <─prev─ B <─prev─ A <─prev─┐
 *     └─next─> A ─next─> B ─next─> head  (circular)
 *
 * Lista vacía: head.next == head.prev == &head
 */

/* ── Macros de utilidad ─────────────────────────────────────────────────── */

/*
 * offsetof — byte de inicio del campo 'member' dentro de 'type'.
 * Equivalente a la macro estándar de <stddef.h>.
 */
#ifndef offsetof
#define offsetof(type, member) \
    ((unsigned int)&((type *)0)->member)
#endif

/*
 * container_of — obtiene el puntero al struct que contiene a 'ptr'.
 *
 * ptr    : puntero al campo 'member' dentro del struct
 * type   : tipo del struct contenedor
 * member : nombre del campo
 */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

/* ── Estructura nodo ────────────────────────────────────────────────────── */

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

/* ── Inicialización ─────────────────────────────────────────────────────── */

/* Valor inicial de un list_head (apunta a sí mismo → lista vacía) */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

/* Declarar e inicializar una cabeza de lista en tiempo de compilación */
#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

/* Inicializar un list_head en tiempo de ejecución */
static inline void list_init(struct list_head *h)
{
    h->next = h;
    h->prev = h;
}

/* ── Operaciones básicas ────────────────────────────────────────────────── */

static inline void _list_insert(struct list_head *new,
                                struct list_head *prev,
                                struct list_head *next)
{
    next->prev = new;
    new->next  = next;
    new->prev  = prev;
    prev->next = new;
}

/* list_add — inserta 'new' justo DESPUÉS de 'head' (semántica de pila, LIFO) */
static inline void list_add(struct list_head *head, struct list_head *new)
{
    _list_insert(new, head, head->next);
}

/* list_add_tail — inserta 'new' justo ANTES de 'head' (semántica de cola, FIFO) */
static inline void list_add_tail(struct list_head *head, struct list_head *new)
{
    _list_insert(new, head->prev, head);
}

/* list_del — elimina 'node' de su lista */
static inline void list_del(struct list_head *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

/* list_empty — 1 si la lista está vacía */
static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

/* ── Acceso al struct contenedor ────────────────────────────────────────── */

/* list_entry — obtiene el struct que contiene 'ptr' */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/* list_first_entry — primer elemento (la lista no debe estar vacía) */
#define list_first_entry(head, type, member) \
    list_entry((head)->next, type, member)

/* list_next_entry — elemento siguiente al 'pos' actual */
#define list_next_entry(pos, type, member) \
    list_entry((pos)->member.next, type, member)

/* ── Iteradores ─────────────────────────────────────────────────────────── */

/* list_for_each — itera sobre los nodos (pos es struct list_head *) */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/* list_for_each_safe — seguro ante borrado durante iteración */
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; \
         pos != (head); \
         pos = n, n = pos->next)

/*
 * list_for_each_entry — itera obteniendo directamente el struct contenedor.
 *
 * pos    : puntero al tipo del struct (se declara antes del bucle)
 * head   : puntero a la cabeza de lista (struct list_head *)
 * type   : tipo del struct que contiene el list_head
 * member : nombre del campo list_head dentro del struct
 *
 * Ejemplo:
 *   task_t *t;
 *   list_for_each_entry(t, &task_list, task_t, list) {
 *       do_something(t);
 *   }
 */
#define list_for_each_entry(pos, head, type, member)                    \
    for (pos = list_first_entry(head, type, member);                    \
         &(pos)->member != (head);                                      \
         pos = list_next_entry(pos, type, member))

#endif
