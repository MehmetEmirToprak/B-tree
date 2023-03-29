#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#define MAXDEG 6

typedef struct BTREE_NODE_s *BTREE_NODE;
typedef struct BTREE_NODE_s{
    unsigned int key[MAXDEG];
    int n;
    int level;
}BTREE_NODE_t[1];

typedef struct {
    int size;
    int max_level;
    BTREE_NODE *tree;
}BTREE_t[1],*BTREE;

BTREE btree_init() {
    BTREE btree = (BTREE) malloc(sizeof(BTREE_t));
    btree->tree = NULL;
    btree->size = 0;
    btree->max_level = 0;
    return btree;
}

BTREE_NODE btree_node_init(int level) {
    BTREE_NODE node = (BTREE_NODE) malloc(sizeof(BTREE_NODE_t));
    node->n = 0;
    node->level = level;
    for(int i = 0; i < MAXDEG; i++) {
        node->key[i] = -1;
    }
    return node;
}

int search(BTREE btree,unsigned int key,int index) {
    int i;
    for(i = 0; i < btree->tree[index]->n; i++) {
        if(btree->tree[index]->key[i] >= key) break;
    }
    return i;
}

void sort(unsigned int key[],int size) {
    int high = 1;
    while(high != 0) {
        high = 0;
        for(int i = 0; i < size - 1; i++) {
            if(key[i] > key[i+1]) {
                unsigned int hold = key[i];
                key[i] = key[i+1];
                key[i+1] = hold;
                high = i+1;
            }
        }
        size = high;
    }
}

void read_file(BTREE btree) {
    FILE *fp = fopen("binary.bin","rb");
    if(!feof(fp) && fp != NULL) {
        fread(&btree->size,sizeof(int),1,fp);
        fread(&btree->max_level,sizeof(int),1,fp);
        int total = 0;
        while(!(feof(fp))) {
            if(btree->tree == NULL) {
                btree->tree = (BTREE_NODE *) malloc(sizeof(BTREE_NODE));
            } else {
                btree->tree = (BTREE_NODE *) realloc(btree->tree,sizeof(BTREE_NODE)*(total+1));
            }
            btree->tree[total] = btree_node_init(1);
            fread(&btree->tree[total]->n,sizeof(int),1,fp);
            fread(&btree->tree[total]->level,sizeof(int),1,fp);
            for(int j = 0; j < btree->tree[total]->n; j++) {
                fread(&btree->tree[total]->key[j],sizeof(unsigned int),1,fp);
            }
            total++;
        }
    }
    fclose(fp);
}

void write_file(BTREE btree) {
    FILE *fp = fopen("binary.bin","wb");
    fwrite(&btree->size,sizeof(int),1,fp);
    fwrite(&btree->max_level,sizeof(int),1,fp);
    int total = 0;
    for(int i = 0; i < btree->size; i++) {
        fwrite(&btree->tree[total]->n,sizeof(int),1,fp);
        fwrite(&btree->tree[total]->level,sizeof(int),1,fp);
        for(int j = 0; j < btree->tree[total]->n; j++) {
            fwrite(&btree->tree[total]->key[j],sizeof(unsigned int),1,fp);
        }
        total++;
    }
    fclose(fp);
}

int find_child(BTREE btree,int index, int pointer) {
    int total_pointer_number = pointer;
    int level = btree->tree[index]->level;

    for(int i = index-1; i >= 0 && btree->tree[i]->level == level; i++)
        total_pointer_number += btree->tree[i]->n+1;

    while(index < btree->size && btree->tree[index]->level == level)
        index++;

    return index+total_pointer_number;
}

bool is_key_founded(BTREE btree, unsigned int key,int index,int i) {
    if(i == MAXDEG)
        return false;
    else {
        if(btree->tree[index]->key[i] == key)
            return true;
        else
            return false;
    }
}

bool is_interval_node(BTREE btree,int index) {
    if(btree->tree[index]->level != 1)
        return true;
    else
        return false;
}

bool is_there_left_child(BTREE btree, int index) {
    if(index-1 >= 0) {
        if (btree->tree[index - 1]->level == btree->tree[index]->level)
            return true;
    }
    return false;
}

bool is_there_key_from_left_child(BTREE btree, int index) {
    int minkey = ceil((double)MAXDEG/2)-1;

    if (btree->tree[index - 1]->n > minkey)
        return true;

    return false;
}

bool is_there_right_child(BTREE btree, int index) {
    if(index+1 < btree->size) {
        if (btree->tree[index + 1]->level == btree->tree[index]->level)
            return true;
    }
    return false;
}

bool is_there_key_from_right_child(BTREE btree, int index) {
    int minkey = ceil((double)MAXDEG/2)-1;

    if (btree->tree[index + 1]->n > minkey)
        return true;

    return false;
}

void get_child(BTREE btree,int index, int parent,int parent_pointer,bool is_right) {
    if(is_right) {
        btree->tree[index]->key[btree->tree[index]->n] = btree->tree[parent]->key[parent_pointer];
        btree->tree[index]->n++;

        btree->tree[parent]->key[parent_pointer] = btree->tree[index+1]->key[0];

        btree->tree[index+1]->key[0] = -1;
        sort(btree->tree[index+1]->key,btree->tree[index+1]->n);
        btree->tree[index+1]->n--;
    } else {
        btree->tree[index]->key[btree->tree[index]->n] = btree->tree[parent]->key[parent_pointer-1];
        btree->tree[index]->n++;

        btree->tree[parent]->key[parent_pointer-1] = btree->tree[index-1]->key[btree->tree[index-1]->n-1];

        btree->tree[index-1]->key[btree->tree[index-1]->n-1] = -1;
        btree->tree[index-1]->n--;
    }
}

void merge(BTREE btree,int index,int parent,int parent_pointer,bool is_right) {
    if(is_right) {
        btree->tree[index]->key[btree->tree[index]->n] = btree->tree[parent]->key[parent_pointer];
        btree->tree[index]->n++;

        btree->tree[parent]->key[parent_pointer] = -1;
        sort(btree->tree[parent]->key,btree->tree[parent]->n);
        btree->tree[parent]->n--;

        for(int i = 0; i < btree->tree[index+1]->n; i++) {
            btree->tree[index]->key[btree->tree[index]->n] = btree->tree[index+1]->key[i];
            btree->tree[index]->n++;
        }

        btree->tree = (BTREE_NODE *) realloc(btree->tree,sizeof(BTREE_NODE)*(btree->size-1));
        memmove(btree->tree+index+1,btree->tree+index+2,sizeof(BTREE_NODE)*(btree->size-index-1));
        btree->size--;
    } else {
        btree->tree[index-1]->key[btree->tree[index-1]->n] = btree->tree[parent]->key[parent_pointer-1];
        btree->tree[index-1]->n++;

        btree->tree[parent]->key[parent_pointer-1] = -1;
        sort(btree->tree[parent]->key,btree->tree[parent]->n);
        btree->tree[parent]->n--;

        for(int i = 0; i < btree->tree[index]->n; i++) {
            btree->tree[index-1]->key[btree->tree[index-1]->n] = btree->tree[index]->key[i];
            btree->tree[index-1]->n++;
        }
        btree->tree = (BTREE_NODE *) realloc(btree->tree,sizeof(BTREE_NODE)*(btree->size-1));
        memmove(btree->tree+index,btree->tree+index+1,sizeof(BTREE_NODE)*(btree->size-index));
        btree->size--;
    }
}

void btree_delete(BTREE btree,unsigned int key,int index,int parent,int parent_pointer,bool was_interval_node) {
    if(index < btree->size) {
        int i = search(btree,key,index);

        if(is_key_founded(btree,key,index,i)) {
            if(is_interval_node(btree,index) && !was_interval_node) {
                int max_child_index = find_child(btree,index,i);
                while(btree->tree[max_child_index]->level != 1) {
                    max_child_index = find_child(btree,max_child_index,btree->tree[max_child_index]->n);
                }
                unsigned int max = btree->tree[max_child_index]->key[btree->tree[max_child_index]->n-1];
                max_child_index = find_child(btree,index,i);
                btree->tree[index]->key[i] = max;
                btree_delete(btree,max,max_child_index,index,i,true);
            } else if(was_interval_node && btree->tree[index]->level != 1){
                btree_delete(btree,key,find_child(btree,index,i),index,i,was_interval_node);
            } else {
                btree->tree[index]->key[i] = -1;
                sort(btree->tree[index]->key,btree->tree[index]->n);
                btree->tree[index]->n--;
            }
        } else {
            btree_delete(btree,key,find_child(btree,index,i),index,i,was_interval_node);
        }
        int min_key = ceil((double)MAXDEG/2)-1;
        if(btree->tree[index]->level != btree->max_level) {
            if(btree->tree[index]->n < min_key) {
                bool control = true;
                if(is_there_left_child(btree,index) && control) {
                    if(is_there_key_from_left_child(btree,index)) {
                        get_child(btree,index,parent,parent_pointer,false);
                        control = false;
                    }
                }
                if(is_there_right_child(btree,index) && control) {
                    if(is_there_key_from_right_child(btree,index)) {
                        get_child(btree,index,parent,parent_pointer,true);
                        control = false;
                    }
                }
                if(control && is_there_left_child(btree,index)) {
                    merge(btree,index,parent,parent_pointer,false);
                    control = false;
                }
                if(control && is_there_right_child(btree,index)) {
                    merge(btree,index,parent,parent_pointer,true);
                }
            }
        } else {
            if(btree->tree[index]->n == 0) {
                memmove(btree->tree+index,btree->tree+index+1,sizeof(BTREE_NODE)*(btree->size-index));
                btree->tree = (BTREE_NODE *) realloc(btree->tree,sizeof(BTREE_NODE)*(btree->size-1));
                btree->size--;
                btree->max_level--;
            }
        }
    }
}

void btree_delete_recursive(BTREE btree,unsigned int key) {
    if(btree == NULL) {
        printf("btree is not initialized!!\n");
    } else {
        if(btree->tree == NULL) {
            printf("btree is empty!!\n");
        } else {
            btree_delete(btree,key,0,0,0,false);
        }
        write_file(btree);
    }
}

void split(BTREE btree,int index,int parent) {
    if(btree->tree[index]->level == btree->max_level) {
        btree->tree = (BTREE_NODE *) realloc(btree->tree,sizeof(BTREE_NODE)*(btree->size+2));

        memmove(btree->tree+3,btree->tree+1,sizeof(BTREE_NODE)*(btree->size-1));

        int middle = (MAXDEG/2);
        BTREE_NODE splinted_node = btree->tree[0];

        btree->tree[0] = btree_node_init(btree->max_level+1);
        btree->tree[1] = btree_node_init(btree->max_level);
        btree->tree[2] = btree_node_init(btree->max_level);

        btree->tree[0]->key[0] = splinted_node->key[middle];
        btree->tree[0]->n++;

        for(int i = 0; i < middle; i++) {
            btree->tree[1]->key[i] = splinted_node->key[i];
        }
        btree->tree[1]->n = middle;

        for(int i = middle+1; i < splinted_node->n; i++) {
            btree->tree[2]->key[i-middle-1] = splinted_node->key[i];
        }
        btree->tree[2]->n = splinted_node->n-middle-1;

        btree->max_level++;
        btree->size += 2;
    } else {
        btree->tree = (BTREE_NODE *) realloc(btree->tree,sizeof(BTREE_NODE)*(btree->size+1));

        memmove(btree->tree+index+2,btree->tree+index+1,sizeof(BTREE_NODE)*(btree->size-index-1));

        int middle = MAXDEG/2;

        btree->tree[index+1] = btree_node_init(btree->tree[index]->level);

        btree->tree[parent]->key[btree->tree[parent]->n] = btree->tree[index]->key[middle];
        btree->tree[index]->key[middle] = -1;
        btree->tree[parent]->n++;
        sort(btree->tree[parent]->key,btree->tree[parent]->n);

        for(int i = middle+1; i < btree->tree[index]->n; i++) {
            btree->tree[index+1]->key[i-middle-1] = btree->tree[index]->key[i];
            btree->tree[index]->key[i] = -1;
        }
        btree->tree[index+1]->n = btree->tree[index]->n-middle-1;

        btree->tree[index]->n = middle;

        btree->size++;
    }
}

void btree_insert(BTREE btree,unsigned int key,int index,int parent) {
    if(index < btree->size) {
        int i = search(btree,key,index);
        if(btree->tree[index]->level == 1) {
            int size = btree->tree[index]->n;
            btree->tree[index]->key[size] = key;
            btree->tree[index]->n++;
            sort(btree->tree[index]->key,btree->tree[index]->n);
        } else {
            btree_insert(btree,key,find_child(btree,index,i),index);
        }
        if(btree->tree[index]->n == MAXDEG) {
            split(btree,index,parent);
        }
    }
}

void btree_insert_recursive(BTREE btree,unsigned int key) {
    if(btree == NULL) {
        printf("btree is not initialized!!\n");
    } else {
        if(btree->tree == NULL) {
            btree->size++;
            btree->max_level++;
            btree->tree = (BTREE_NODE *) malloc(sizeof(BTREE_NODE));
            btree->tree[0] = btree_node_init(1);
            btree->tree[0]->key[0] = key;
            btree->tree[0]->n = 1;
        } else {
            btree_insert(btree,key,0,0);
        }
        write_file(btree);
    }
}

void print_btree(BTREE btree) {
    int level = btree->max_level;
    for(int i = 0; i < btree->size; i++) {
        if(btree->tree[i]->level != level) {
            printf("\n");
            level = btree->tree[i]->level;
        }
        for(int j = 0; j < btree->tree[i]->n; j++) {
            printf("%u ",btree->tree[i]->key[j]);
        }
        printf(" ");
    }
}

int main() {
    BTREE btree = btree_init();
    read_file(btree);
    for(unsigned int i = 0; i < 10; i++) {
        btree_insert_recursive(btree,i);
        print_btree(btree);
    }
    for(unsigned int i = 0; i < 5; i++) {
        btree_delete_recursive(btree,i);
        print_btree(btree);
    }
    return 0;
}
