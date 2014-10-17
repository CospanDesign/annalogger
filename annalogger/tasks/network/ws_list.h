//web socket list.h

#ifndef __CSPND_WS_LIST
#define __CSPND_WS_LIST

#include <stdint.h>
#include <stdbool.h>


typedef struct _wsl_head_t wsl_head_t;
typedef struct _wsl_node_t wsl_node_t;

//wsl of pointers
struct _wsl_node_t
{
    wsl_node_t *prev;
    wsl_node_t *next;

    uint16_t id;
    void * data;
};

struct _wsl_head_t
{
    int count;
    wsl_node_t *start;
    wsl_node_t *end;
    char *name;
};

//initialize an empty web socket list with the given name
wsl_head_t * wsl_new (char *name);
//add a node to the web socket list with a id (string), and data (void pointer)
bool wsl_add (wsl_head_t *lh, uint16_t id, void * data);
//remove a node in the web socket list with a given index
bool wsl_remove (wsl_head_t *lh, unsigned int index);
//destroy an entire wsl
void wsl_destroy (wsl_head_t *lh);
//search the web socket list for a id, if found return the index, otherwise return -1
int wsl_get_index_from_id (wsl_head_t *lh, uint16_t id);
//get the data from the web socket list from an index, return NULL if error
void * wsl_get_data_from_index (wsl_head_t *lh, unsigned int index);
//get the data from the web socket list from a id, return NULL if error
void * wsl_get_data_from_id (wsl_head_t *lh, uint16_t id);
//get index from comparing the data pointers
int wsl_get_index_from_data (wsl_head_t *lh, void * data);
//get the web socket list size
int wsl_get_size(wsl_head_t *lh);

//set id
bool wsl_set_id_by_id (wsl_head_t *lh, uint16_t idIn, uint16_t idSearch);
bool wsl_set_id_by_index (wsl_head_t *lh, uint16_t id, unsigned int index);

//get id
uint16_t wsl_get_id_from_index (wsl_head_t *lh, unsigned int index);

//set data
bool wsl_set_data_by_id (wsl_head_t *lh, uint16_t id, void * data);
bool wsl_set_data_by_index (wsl_head_t *lh, unsigned int index, void * data);

#endif
