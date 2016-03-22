#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MIN_MAP_SIZE 8
#define MAX_STR_SIZE 256

struct hashmap_item {
    uint32_t hash;
    char *key;
    char *value;
    struct hashmap_item *next;
};

struct hashmap {
  size_t size;
  size_t count;
  struct hashmap_item **buckets;
};

bool hashmap_view(struct hashmap *map);
bool hashmap_resize(struct hashmap *map);


uint32_t hashmap_hash(char *key) {
    uint32_t hash, i;
    for(hash = i = 0; i < strlen(key); ++i) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

uint32_t hashmap_index(uint32_t h, size_t size) {
    // printf("SIZE: %u %lu %lu\n", h, size, h & (size - 1));
    return h & (size - 1);
}

struct hashmap* hashmap_new() {
    struct hashmap *map = malloc(sizeof(struct hashmap));
    if(map == NULL) {
        return NULL;
    }
    map->buckets =
      malloc(MIN_MAP_SIZE * sizeof(struct hashmap_bucket*));
    if(map->buckets == NULL) {
        free(map);
        return NULL;
    }

    memset(map->buckets, 0, sizeof(struct hashmap_bucket*) * MIN_MAP_SIZE);
    map->size = MIN_MAP_SIZE;
    map->count = 0;

    return map;
}

bool hashmap_put(struct hashmap *map, char *key, char *value) {
    hashmap_resize(map);

    size_t key_len = strlen(key) + 1;
    size_t value_len = strlen(value) + 1;

    uint32_t h = hashmap_hash(key);
    uint32_t index = hashmap_index(h, map->size);
    struct hashmap_item *item = map->buckets[index];

    while(item) {
        if (strcmp(item->key, key) == 0) {
            char *tmp = malloc(value_len);
            if (tmp == NULL) {
                // malloc error
                return false;
            }
            memcpy(tmp, value, value_len);
            free(item->value);
            item->value = tmp;
            return true;
        }
        item = item->next;
    }

    item = malloc(sizeof(struct hashmap_item));
    if (item == NULL) {
        // malloc error
        return false;
    }

    item->value = malloc(value_len);
    if (item->value == NULL) {
        // malloc error
        free(item);
        return false;
    }
    memcpy(item->value, value, value_len);

    item->key = malloc(key_len);
    if (item->key == NULL) {
        // malloc error
        free(item->value);
        free(item);
        return false;
    }
    memcpy(item->key, key, key_len);

    item->hash = h;
    item->next = map->buckets[index];
    map->buckets[index] = item;
    ++(map->count);

    return true;
}

char *hashmap_get(struct hashmap *map, char *key) {
    size_t key_len = strlen(key) + 1;

    uint32_t index = hashmap_index(hashmap_hash(key), map->size);
    struct hashmap_item *item = map->buckets[index];

    while(item) {
        if (strcmp(item->key, key) == 0) {
            return item->value;
        }
        item = item->next;
    }

    return NULL;
}

bool hashmap_delete(struct hashmap *map, char *key) {
    size_t key_len = strlen(key) + 1;

    uint32_t index = hashmap_index(hashmap_hash(key), map->size);
    struct hashmap_item *item = map->buckets[index];
    struct hashmap_item *pr = NULL;

    while(item) {
        if (strcmp(item->key, key) == 0) {
            if (pr) {
                pr->next = item->next;
            } else {
                map->buckets[index] = item->next;
            }
            free(item->key);
            free(item->value);
            free(item);

            --(map->count);
            return true;
        }
        pr = item;
        item = item->next;
    }

    return false;    
}

bool hashmap_resize(struct hashmap *map) {
    if (map->count >= map->size) {
        size_t old_size = map->size;
        size_t new_size = old_size << 1;
    
        struct hashmap_item **new_buckets =
          malloc(new_size * sizeof(struct hashmap_bucket*));
        if(new_buckets == NULL) {
            return false;
        }

        memset(new_buckets, 0, new_size * sizeof(struct hashmap_bucket*));
        memcpy(new_buckets, map->buckets, old_size * sizeof(struct hashmap_bucket*));

        size_t i;
        struct hashmap_item *pr, *cur, *tmp;
        for (i = 0; i < old_size; ++i) {
            pr = NULL;
            cur = new_buckets[i];
            while (cur) {
                if (cur->hash & old_size) {
                    tmp = cur;
                    if (pr) {
                        pr->next = cur->next;
                        cur = cur->next;
                    } else {
                        new_buckets[i] = cur->next;
                        cur = cur->next;
                    }

                    size_t index = hashmap_index(tmp->hash, new_size);
                    // printf("%lu : %lu : %s\n", i, index, tmp->key);
                    tmp->next = new_buckets[index];
                    new_buckets[index] = tmp;
                    // printf("NEW: %s\n", new_buckets[index]->key);
                } else {
                    pr = cur;
                    cur = cur->next;
                }
            }
        }

        free(map->buckets);
        map->buckets = new_buckets;
        map->size = new_size;

        return true;
    }

    return false;
}

bool hashmap_view(struct hashmap *map) {
    struct hashmap_item *item;
    int i;

    for (i = 0; i < map->size; ++i) {
        item = map->buckets[i];
        if (item) {
            printf("%u: ", i);
            while (item) {
                printf("%u:%s:%s ", item->hash, item->key, item->value);
                item = item->next;
            }
            printf("\n");
        } else {
            printf("%u: null\n", i);
        }
    }
}

int main()
{
    size_t len;
    char str[MAX_STR_SIZE], key[MAX_STR_SIZE], value[MAX_STR_SIZE];
    char *token;

    struct hashmap* map = hashmap_new();

    while (fgets(str, MAX_STR_SIZE - 1, stdin) != NULL) {
        len = strlen(str); 
        if ((len > 0) && (str[len - 1] == '\n'))
            str[len - 1] = '\0';

        switch(str[0]) {
            case '*':
                return 0;
                break;
            case '+':
                if (str[1] != ' ') {
                    break;
                }

                token = strtok(str, " ");
                if (token == NULL) {
                    break;
                }

                token = strtok(NULL, " ");
                if (token == NULL) {
                    break;
                }
                strcpy(key, token);
                // printf("KEY: %s %s\n", token, key);

                token = strtok(NULL, " ");
                if (token == NULL) {
                    break;
                }
                strcpy(value, token);
                // printf("VALUE: %s %s\n", token, value);

                hashmap_put(map, key, value);

                break;
            case '-':
                if (str[1] != ' ') {
                    break;
                }

                token = strtok(str, " ");
                if (token == NULL) {
                    break;
                }

                token = strtok(NULL, " ");
                if (token == NULL) {
                    break;
                }
                strcpy(key, token);

                hashmap_delete(map, key);

                break;
            default:
                token = hashmap_get(map, str);

                if (token != NULL) {
                    printf("%s\n", token);
                }

                break;
        }
    }

    return 0;
}
