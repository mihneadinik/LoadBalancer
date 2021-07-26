#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LinkedList.h"
#include "utils.h"

linked_list_t*
ll_create(unsigned int data_size)
{
    linked_list_t *list = malloc(sizeof(linked_list_t));
    DIE(list == NULL, "Eroare");
    list->size = 0;
    list->data_size = data_size;
    list->head = NULL;
    return list;
}

/*
 * Pe baza datelor trimise prin pointerul new_data, se creeaza un nou nod care e
 * adaugat pe pozitia n a listei reprezentata de pointerul list. Pozitiile din
 * lista sunt indexate incepand cu 0 (i.e. primul nod din lista se afla pe
 * pozitia n=0). Daca n >= nr_noduri, noul nod se adauga la finalul listei. Daca
 * n < 0, eroare.
 */
void
ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    if (list == NULL)
        return;
    DIE(n < 0, "Eroare");
    ll_node_t *new;
    new = malloc(sizeof(ll_node_t));
    DIE(new == NULL, "Eroare");
    new->data = malloc(list->data_size);
    memcpy(new->data, new_data, list->data_size);
    int nodes_nr = ll_get_size(list);
    if (n >= list->size)
        n = list->size;
    if (n == 0) {
        new->next = list->head;
        list->head = new;
        list->size ++;
    } else {
        ll_node_t *curr;
        curr = list->head;
        for (int i = 1; i < n; i++)
            curr = curr->next;
        new->next = curr->next;
        curr->next = new;
        list->size ++;
    }

}

/*
 * Elimina nodul de pe pozitia n din lista al carei pointer este trimis ca
 * parametru. Pozitiile din lista se indexeaza de la 0 (i.e. primul nod din
 * lista se afla pe pozitia n=0). Daca n >= nr_noduri - 1, se elimina nodul de
 * la finalul listei. Daca n < 0, eroare. Functia intoarce un pointer spre acest
 * nod proaspat eliminat din lista. Este responsabilitatea apelantului sa
 * elibereze memoria acestui nod.
 */
ll_node_t*
ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    if (list == NULL || list->head == NULL)
        exit;
    DIE(n < 0, "Eroare");
    int nodes_nr = ll_get_size(list);
    ll_node_t *out;
    if(list->head->next == NULL) {
        out = list->head->next;
        list->head = NULL;
        return out;
    }
    if (n >= list->size - 1)
        n = list->size - 1;
    if (n == 0) {
        out = list->head;
        list->head = list->head->next;
        list->size --;
        return out;
    } else {
        ll_node_t *last, *prev;
        last = list->head->next;
        prev = list->head;
        for (int i = 1; i < n; i++) {
            prev = last;
            last = last->next;
        }
        prev->next = last->next;
        out = last;
        list->size --;
        return out;
    }
}

/*
 * Functia intoarce numarul de noduri din lista al carei pointer este trimis ca
 * parametru.
 */
unsigned int
ll_get_size(linked_list_t* list)
{
    DIE(list == NULL, "Empty list");
    return list->size;
}

/*
 * Procedura elibereaza memoria folosita de toate nodurile din lista, iar la
 * sfarsit, elibereaza memoria folosita de structura lista si actualizeaza la
 * NULL valoarea pointerului la care pointeaza argumentul (argumentul este un
 * pointer la un pointer).
 */
void
ll_free(linked_list_t** pp_list)
{
    if (*pp_list == NULL)
        return;
    if ((*pp_list)->head == NULL) {
        free(*pp_list);
        return;
    }
    ll_node_t *last, *prev;
    last = (*pp_list)->head->next;
    prev = (*pp_list)->head;
    while(last != NULL) {
        free(prev->data);
        free(prev);
        prev = last;
        last = last->next;
    }
    free(prev->data);
    free(prev);
    free(*pp_list);
    *pp_list = NULL;
}

/*
 * Atentie! Aceasta functie poate fi apelata doar pe liste ale caror noduri STIM
 * ca stocheaza int-uri. Functia afiseaza toate valorile int stocate in nodurile
 * din lista inlantuita separate printr-un spatiu.
 */
void
ll_print_int(linked_list_t* list)
{
    DIE(list == NULL, "Eroare");
    ll_node_t *curr;
    if (list == NULL)
        return;
    curr = list->head;
    while (curr != NULL) {
        printf("%d ", *(int*)(curr->data));
        curr = curr->next;
    }
    printf("\n");
}

/*
 * Atentie! Aceasta functie poate fi apelata doar pe liste ale caror noduri STIM
 * ca stocheaza string-uri. Functia afiseaza toate string-urile stocate in
 * nodurile din lista inlantuita, separate printr-un spatiu.
 */
void
ll_print_string(linked_list_t* list)
{
    DIE(list == NULL, "Eroare");
    ll_node_t *curr;
    if (list == NULL)
        return;
    curr = list->head;
    while (curr != NULL) {
        int i = 0;
        while(*((char*)(curr->data) + i) != '\0') {
            printf("%c", *((char*)(curr->data) + i));
            i++;
        }
        printf(" ");
        curr = curr->next;
    }
    printf("\n");
}

void
ll_merge(linked_list_t *list1, linked_list_t *list2)
{
    DIE(list1 == NULL || list2 == NULL, "Eroare");
    ll_node_t *curr1, *curr2;
    int l1 = list1->size, l2 = list2->size;
    DIE(l1 != l2, "Listele nu sunt egale");
    DIE(list1->data_size != list2->data_size, "Listele nu sunt de acelasi tip");
    list1->size = 2 * l1;
    int i, j;
    while(l2 != 0) {
        curr1 = list1->head;
        curr2 = list2->head;
        for (i = 1; i < l1; i++)
            curr1 = curr1->next;
        for (j = 1; j < l2; j++)
            curr2 = curr2->next;
        curr1->next = curr2;
        l2--;
        if (l2 == 0)
            break;
        curr1 = list1->head;
        curr2 = list2->head;
        for (i = 1; i < l1; i++)
            curr1 = curr1->next;
        for (j = 1; j < l2; j++)
            curr2 = curr2->next;
        curr2->next = curr1;
        l1--;
    }
    list2->size = 0;
    list2->head = NULL;
}
