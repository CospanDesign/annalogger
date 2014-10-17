#include "ws_list.h"
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//initialize web socket list
wsl_head_t * wsl_new (char *name){
    //make a new web socket list

    //get a poitner
    wsl_head_t *lh = NULL;

    //attach the poitner to a allocated structure
    lh = calloc (1, sizeof (wsl_head_t));

    if (lh == NULL){
        //FAILED!
        return NULL;
    }

    //give the web socket list a name
    lh->name = calloc(strlen(name), sizeof(char));
    if (lh->name == NULL){
      free(lh);
      return NULL;
    }
    //lh->name = strdup(name);
    strcpy(lh->name, name);
    //no items in wsl, so set the count = 0;
    lh->count = 0;

    lh->start = NULL;
    lh->end = NULL;

    //return the new web socket list head
    return lh;
}
//get web socket list size
int wsl_get_size(wsl_head_t *lh){
  return lh->count;
}

//add item to the web socket list
bool wsl_add (wsl_head_t *lh, uint16_t id, void * data){
    if (lh == NULL){
        return false;
    }

    //create a pointer to a new node
    wsl_node_t *ln = NULL;

    ln = calloc (1, sizeof (wsl_node_t));

    if (ln == NULL){
        //failed
        return false;
    }

    ln->id = id;
    //set up the data
    ln->data = data;

    //if we don't have anything within the web socket list we need to create a new node to attach to it
    if (lh->count == 0){
        //set the start, and end pointers equal to the first node
        lh->start = ln;
        lh->end = ln;
        lh->count = 1;

        //set up the node to point to itself!
        ln->prev = NULL;
        ln->next = NULL;
    }
    //we already had some nodes in the structure
    else
    {
        //new node previous points to the old last node in the web socket list
        ln->prev = lh->end;
        //new node now points to NULL;
        ln->next = NULL;

        //set up the previous last node to point to the new last node
        lh->end->next = ln;
        lh->end = ln;
        lh->count ++;

    }

    //return true
    return true;
}

//remove an item from the web socket list
bool wsl_remove (wsl_head_t *lh, unsigned int index){
    wsl_node_t *ln;

    //check to see if the head exists
    if (lh == NULL){
        return false;
    }

    //index doesn't exist within the array
    if (index + 1 > lh->count){
        return false;
    }

    //if we have only one node
    if (lh->count == 1){
        lh->count = 0;
        ln = lh->start;
        free (ln);
        lh->start = NULL;
        lh->end = NULL;
        return true;
    }

    ln = lh->start;
    //go through the points until we find our node
    for (int i = 0; i < index ; i++){
        ln = ln->next;
    }

    //found our node... start attaching pointers

    //if we are at the start
    if (lh->start == ln){
        //attach head start to the next node
        lh->start = lh->start->next;
        //set the prev node to NULL
        lh->start->prev = NULL;
        free (ln);
        lh->count--;
        return true;
    }
    //if we are at the end
    else if (lh->end == ln){
        //attach the head end to the previous node
        lh->end = lh->end->prev;
        //set the next node to NULL;
        lh->end->next = NULL;
        free (ln);
        lh->count--;
        return true;
    }
    //anywhere else

    //tell the prev node to go to the next node
    ln->prev->next = ln->next;
    //tell the next node to go to the prev node
    ln->next->prev = ln->prev;
    free (ln);
    lh->count--;
    return true;

}
//destroy web socket list
void wsl_destroy (wsl_head_t *lh){
    if (lh == NULL){
        return;
    }
    //kill off all nodes
    while (lh->count > 1){
        //move the web socket list head end pointer a node back
        lh->end = lh->end->prev;

        //the head head of the web socket list points to the previous node,
        //but the previous node should still point to the now dangaling node
        //free that
        free (lh->end->next);
        lh->count--;
        //now set the last node's next equal to NULL
        lh->end->next = NULL;
    }

    if (lh->count == 1){
        free (lh->start);

    }

    //everything is free!
    free (lh->name);
    free (lh);
}

int wsl_get_index_from_id (wsl_head_t *lh, uint16_t id){
    int index = 0;
    wsl_node_t * ln;

    //if we don't have anything in the web socket list bail
    if (lh->count == 0){
        return -1;
    }

    //set up the node to attach to the first node
    ln = lh->start;

    //while we haven't found a match
    while (id != ln->id){
        index++;
        //are we at the end?
        if (ln->next == NULL){
            return -1;
        }

        //go to the next node
        ln = ln->next;
    }

    //return the index we found
    return index;
}

void * wsl_get_data_from_index (wsl_head_t *lh, unsigned int index){
    wsl_node_t *ln;

    //check to see if the head exists
    if (lh == NULL){
        return NULL;
    }

    //index doesn't exist within the array
    if (index + 1 > lh->count){
        return NULL;
    }

    //this next line doesn't work because when we add a new node we don't know where it will be... if we knew it was consequtive we would be fine
    //ln = lh->start + (index * sizeof(wsl_node_t));

    ln = lh->start;

    //go through the points until we find our node

    for (int i = 0; i < index ; i++){
        ln = ln->next;
    }

    //we are at the correct location
    return ln->data;
}

void * wsl_get_data_from_id (wsl_head_t *lh, uint16_t id){
    int index = 0;
    wsl_node_t * ln;

    //if we don't have anything in the web socket list bail
    if (lh->count == 0){
        return NULL;
    }

    //set up the node to attach to the first node
    ln = lh->start;

    //while we haven't found a match
    while (id != ln->id){
        index++;
        //are we at the end?
        if (ln->next == NULL){
            return NULL;
        }

        //go to the next node
        ln = ln->next;
    }
    return ln->data;
}

//set id
bool wsl_set_id_by_id (wsl_head_t *lh, uint16_t idIn, uint16_t idSearch){
    int index = 0;
    wsl_node_t * ln;
    //char * cPtr;

    //if we don't have anything in the web socket list bail
    if (lh->count == 0){
        return false;
    }

    //set up the node to attach to the first node
    ln = lh->start;

    //while we haven't found a match
    while (idIn != ln->id){
        index++;
        //are we at the end?
        if (ln->next == NULL){
            return false;
        }

        //go to the next node
        ln = ln->next;
    }

    //at the correct node

    //replace
    ln->id = idIn;

    return true;
}

bool wsl_set_id_by_index (wsl_head_t *lh, uint16_t id, unsigned int index){
    wsl_node_t *ln;
    //char * cPtr;

    //check to see if the head exists
    if (lh == NULL){
        return false;
    }

    //index doesn't exist within the array
    if (index + 1 > lh->count){
        return false;
    }

    ln = lh->start;
    //go through the points until we find our node
    for (int i = 0; i < index ; i++){
        ln = ln->next;
    }
    //at the correct node
    
    //replace
    ln->id = id;

    return true;
}

uint16_t wsl_get_id_from_index (wsl_head_t *lh, unsigned int index){
    wsl_node_t *ln;

    //check to see if the head exists
    if (lh == NULL){
        return 0;
    }

    //index doesn't exist within the array
    if (index + 1 > lh->count){
        return 0;
    }

    ln = lh->start;
    //go through the points until we find our node
    for (int i = 0; i < index ; i++){
        ln = ln->next;
    }

    //at the correct node

    return ln->id;
}

bool wsl_set_data_by_id (wsl_head_t *lh, uint16_t id, void * data){
    int index = 0;
    wsl_node_t * ln;

    //if we don't have anything in the web socket list bail
    if (lh->count == 0){
        return false;
    }

    //set up the node to attach to the first node
    ln = lh->start;

    //while we haven't found a match
    while (id != ln->id){
        index++;
        //are we at the end?
        if (ln->next == NULL){
            return false;
        }

        //go to the next node
        ln = ln->next;
    }

    ln->data = data;

    //at the node
    return true;
}

bool wsl_set_data_by_index (wsl_head_t *lh, unsigned int index, void * data){
    wsl_node_t *ln;

    //check to see if the head exists
    if (lh == NULL){
        return false;
    }

    //index doesn't exist within the array
    if (index + 1 > lh->count){
        return false;
    }

    ln = lh->start;
    //go through the points until we find our node
    for (int i = 0; i < index ; i++){
        ln = ln->next;
    }

    ln->data = data;

    return true;
}

//compare the data pointer with the pointer at each index, and if the data matches, return index, otherwise return -1;
int wsl_get_index_from_data (wsl_head_t *lh, void * data){
    int index = 0;
    wsl_node_t *ln = lh->start;

    //spin through the web socket list looking for data, if found return the index, otherwise we hit NULL, and give an ERROR
    while (ln->data != data){
        index++;
        //hit the end of the web socket list
        if (ln->next == NULL){
            //send an error
            return -1;
        }
        ln = ln->next;
    }

    //found it
    return index;
}

