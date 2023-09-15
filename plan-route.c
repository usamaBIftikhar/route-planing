#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#define MIN_HEAP_CAPACITY 100

typedef struct MaxHeapAuto {
    int size;
    int array_auto[512];
} MaxHeapAuto;

typedef struct BSTNode {
    int distance;
    MaxHeapAuto* auto_heap;
    struct BSTNode* left;
    struct BSTNode* right;
    int distance_dijkstra;
    int sum_distances_from_zero ;
    struct BSTNode* prev;
} BSTNode;

typedef struct MinHeapNode {
    int distance_dijkstra;
    int sum_distances_from_zero;
    BSTNode* bst_node;
} MinHeapNode;

typedef struct MinHeap {
    int size;
    int capacity;
    MinHeapNode** array;
} MinHeap;

void max_heapify(MaxHeapAuto* heap, int idx);

BSTNode* create_bst_node(int distance) {
    BSTNode* new_node = (BSTNode*)malloc(sizeof(BSTNode));

    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    new_node->distance = distance;
    new_node->auto_heap = NULL;
    new_node->left = NULL;
    new_node->right = NULL;

    // Inizializzazione dei campi per Dijkstra
    new_node->distance_dijkstra = INT_MAX;
    new_node->sum_distances_from_zero = 0;
    new_node->prev = NULL;

    return new_node;
}

void reset_dijkstra_values(BSTNode* node) {
    if (node == NULL) {
        return;
    }
    node->distance_dijkstra = INT_MAX;
    node->sum_distances_from_zero = 0;
    node->prev = NULL;
    reset_dijkstra_values(node->left);
    reset_dijkstra_values(node->right);
}

BSTNode* search_station(BSTNode* root, int distance) {
    // Caso base: l'albero è vuoto o abbiamo trovato la stazione
    if (root == NULL || root->distance == distance) {
        return root;
    }

    // Altrimenti, ricorre sul sottalbero appropriato
    if (distance < root->distance) {
        return search_station(root->left, distance);
    } else {
        return search_station(root->right, distance);
    }
}

// Funzione ausiliaria per trovare il nodo con il valore minimo
// (usata per trovare il successore in ordine di un nodo)
BSTNode* minValueNode(BSTNode* node) {
    BSTNode* current = node;

    // Scorri fino a trovare il nodo più a sinistra (il più piccolo)
    while (current && current->left != NULL) {
        current = current->left;
    }

    return current;
}

BSTNode* delete_station(BSTNode* root, int distance, int* is_removed) {
    // Caso base: l'albero è vuoto
    if (root == NULL) {
        *is_removed = 0;  // Imposta il flag per indicare che la stazione NON è stata rimossa
        return root;
    }

    // Ricorre sul sottalbero appropriato per trovare la stazione da rimuovere
    if (distance < root->distance) {
        root->left = delete_station(root->left, distance, is_removed);
    } else if (distance > root->distance) {
        root->right = delete_station(root->right, distance, is_removed);
    } else {
        // Nodo con solo un figlio o nessun figlio
        if (root->left == NULL) {
            BSTNode* temp = root->right;
            if (root->auto_heap != NULL) {
                free(root->auto_heap);
            }
            free(root);
            *is_removed = 1;  // Imposta il flag per indicare che la stazione è stata rimossa
            return temp;
        } else if (root->right == NULL) {
            BSTNode* temp = root->left;
            if (root->auto_heap != NULL) {
                free(root->auto_heap);
            }
            free(root);
            *is_removed = 1;  // Imposta il flag per indicare che la stazione è stata rimossa
            return temp;
        }

        // Nodo con due figli: trova il successore in ordine (il più piccolo
        // nodo nel sottoalbero destro)
        BSTNode* temp = minValueNode(root->right);

        // Libera l'auto_heap corrente se esiste
        if (root->auto_heap != NULL) {
            free(root->auto_heap);
        }

        // Copia il contenuto del successore in ordine nel nodo
        root->distance = temp->distance;
        root->auto_heap = temp->auto_heap;

        // Imposta l'auto_heap del nodo da eliminare a NULL per evitare deallocazione
        temp->auto_heap = NULL;  // Aggiungi questa linea

        // Rimuovi il successore in ordine
        root->right = delete_station(root->right, temp->distance, is_removed);
    }

    return root;
}

MaxHeapAuto* create_max_heap_auto() {
    MaxHeapAuto* heap = (MaxHeapAuto*)malloc(sizeof(MaxHeapAuto));

    if (heap == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    heap->size = 0;

    return heap;
}

void insert_auto(BSTNode* root, int distance, int autonomy, int* is_added) {
    BSTNode* station = search_station(root, distance);

    if (station == NULL) {
        *is_added = 0;  // Imposta il flag per indicare che l'auto NON è stata aggiunta
        return;
    }

    MaxHeapAuto* heap = station->auto_heap;

    // Inserisce la nuova auto alla fine del heap
    heap->size++;
    int i = heap->size - 1;
    heap->array_auto[i] = autonomy;

    // Sposta la nuova auto alla posizione corretta nel heap
    while (i != 0 && heap->array_auto[(i - 1) / 2] < heap->array_auto[i]) {
        int temp = heap->array_auto[i];
        heap->array_auto[i] = heap->array_auto[(i - 1) / 2];
        heap->array_auto[(i - 1) / 2] = temp;
        i = (i - 1) / 2;
    }

    *is_added = 1;  // Imposta il flag per indicare che l'auto è stata aggiunta
}

// Funzione ausiliaria per mantenere la proprietà di max heap
void max_heapify(MaxHeapAuto* heap, int idx) {
    int largest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && heap->array_auto[left] > heap->array_auto[largest]) {
        largest = left;
    }
    if (right < heap->size && heap->array_auto[right] > heap->array_auto[largest]) {
        largest = right;
    }

    if (largest != idx) {
        int temp = heap->array_auto[idx];
        heap->array_auto[idx] = heap->array_auto[largest];
        heap->array_auto[largest] = temp;
        max_heapify(heap, largest);
    }
}

void remove_auto(BSTNode* root, int distance, int autonomy, int* is_removed) {
    // Cerca la stazione con la distanza specificata nel BST
    BSTNode* station = search_station(root, distance);

    if (station == NULL) {
        *is_removed = 0;  // Imposta il flag per indicare che l'auto NON è stata rimossa
        return;
    }

    MaxHeapAuto* heap = station->auto_heap;

    // Cerca l'auto con l'autonomia specificata
    int i;
    for (i = 0; i < heap->size; i++) {
        if (heap->array_auto[i] == autonomy) {
            break;
        }
    }

    if (i == heap->size) {
        *is_removed = 0;  // Imposta il flag per indicare che l'auto NON è stata rimossa
        return;
    }

    // Sostituisce l'auto con l'ultima auto nel heap
    heap->array_auto[i] = heap->array_auto[heap->size - 1];
    heap->size--;

    // Ripristina la proprietà di max heap
    max_heapify(heap, i);

    *is_removed = 1;  // Imposta il flag per indicare che l'auto è stata rimossa
}

int get_max_autonomy_auto(MaxHeapAuto* heap) {
    if (heap->size == 0) {
        return -1;  // Indica che la coda di priorità è vuota
    }

    return heap->array_auto[0];
}

BSTNode* add_car(BSTNode* root, int distance, int num_auto, int autonomies[], int* is_station_added) {
    // Inizio codice di insert_station
    if (root == NULL) {
        *is_station_added = 1;
        root = create_bst_node(distance);

        // Utilizza il riferimento alla nuova stazione per creare il max_heap e inserire le auto
        root->auto_heap = create_max_heap_auto();
        MaxHeapAuto* heap = root->auto_heap;

        for (int i = 0; i < num_auto; i++) {
            // Inserisce la nuova auto alla fine del heap
            heap->size++;
            int j = heap->size - 1;
            heap->array_auto[j] = autonomies[i];

            // Sposta la nuova auto alla posizione corretta nel heap
            while (j != 0 && heap->array_auto[(j - 1) / 2] < heap->array_auto[j]) {
                int temp = heap->array_auto[j];
                heap->array_auto[j] = heap->array_auto[(j - 1) / 2];
                heap->array_auto[(j - 1) / 2] = temp;
                j = (j - 1) / 2;
            }
        }

    } else {
        if (distance < root->distance) {
            root->left = add_car(root->left, distance, num_auto, autonomies, is_station_added);
        } else if (distance > root->distance) {
            root->right = add_car(root->right, distance, num_auto, autonomies, is_station_added);
        } else {
            *is_station_added = 0;
            return root;
        }
    }

    return root;
}

void resize_min_heap(MinHeap* heap) {
    heap->capacity *= 2;
    heap->array = realloc(heap->array, heap->capacity * sizeof(MinHeapNode*));
    if (heap->array == NULL) {
        // Gestisci l'errore di allocazione della memoria, ad esempio terminando il programma
        printf("Memory allocation failed\n");
        exit(1);
    }
}

void insert_in_minHeap(MinHeap* heap, BSTNode* bst_node, int distance, int sum_distances_from_zero) {
    // Controlla se il heap è pieno e, in tal caso, ridimensiona
    if (heap->size == heap->capacity) {
        resize_min_heap(heap);
    }

    MinHeapNode* new_heap_node = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    if (new_heap_node == NULL) {
        // Gestisci l'errore di allocazione della memoria
        printf("Memory allocation failed\n");
        exit(1);
    }

    new_heap_node->bst_node = bst_node;
    new_heap_node->distance_dijkstra = distance;
    new_heap_node->sum_distances_from_zero = sum_distances_from_zero;

    heap->array[heap->size] = new_heap_node;
    heap->size++;

    int idx = heap->size - 1;
    while (idx) {
        int parent_idx = (idx - 1) / 2;
        if (heap->array[idx]->distance_dijkstra < heap->array[parent_idx]->distance_dijkstra ||
            (heap->array[idx]->distance_dijkstra == heap->array[parent_idx]->distance_dijkstra &&
             heap->array[idx]->sum_distances_from_zero < heap->array[parent_idx]->sum_distances_from_zero)) {
            // Swap
            MinHeapNode* temp = heap->array[idx];
            heap->array[idx] = heap->array[parent_idx];
            heap->array[parent_idx] = temp;
            idx = parent_idx;
        } else {
            break;
        }
    }
}

void min_heapify(MinHeap* heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    // Utilizzare il nuovo campo nel confronto
    if (left < heap->size) {
        if (heap->array[left]->distance_dijkstra < heap->array[smallest]->distance_dijkstra ||
            (heap->array[left]->distance_dijkstra == heap->array[smallest]->distance_dijkstra &&
             heap->array[left]->sum_distances_from_zero < heap->array[smallest]->sum_distances_from_zero)) {
            smallest = left;
        }
    }

    if (right < heap->size) {
        if (heap->array[right]->distance_dijkstra < heap->array[smallest]->distance_dijkstra ||
            (heap->array[right]->distance_dijkstra == heap->array[smallest]->distance_dijkstra &&
             heap->array[right]->sum_distances_from_zero < heap->array[smallest]->sum_distances_from_zero)) {
            smallest = right;
        }
    }

    if (smallest != idx) {
        // Swap the nodes
        MinHeapNode* temp = heap->array[smallest];
        heap->array[smallest] = heap->array[idx];
        heap->array[idx] = temp;

        min_heapify(heap, smallest);
    }
}

void resize_down_min_heap(MinHeap* heap) {
    heap->capacity /= 2;
    heap->array = (MinHeapNode**)realloc(heap->array, heap->capacity * sizeof(MinHeapNode*));
    if (heap->array == NULL) {
        // Gestisci l'errore di allocazione della memoria, ad esempio terminando il programma
        printf("Memory allocation failed\n");
        exit(1);
    }
}

MinHeapNode* extract_min(MinHeap* heap) {
    if (heap->size == 0) return NULL;

    MinHeapNode* root = heap->array[0];
    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;
    min_heapify(heap, 0);

    return root;
}

void decrease_key(MinHeap* heap, BSTNode* bst_node, int distance) {
    // Find the node's index in the heap array
    int i;
    for (i = 0; i < heap->size; i++) {
        if (heap->array[i]->bst_node == bst_node) break;
    }

    // Update the distance and sum of distances to zero
    heap->array[i]->distance_dijkstra = distance;
    heap->array[i]->bst_node->sum_distances_from_zero = bst_node->sum_distances_from_zero;

    // Move the updated node up the heap to maintain the heap property
    while (i && (heap->array[i]->distance_dijkstra < heap->array[(i - 1) / 2]->distance_dijkstra ||
                 (heap->array[i]->distance_dijkstra == heap->array[(i - 1) / 2]->distance_dijkstra &&
                  heap->array[i]->bst_node->sum_distances_from_zero < heap->array[(i - 1) / 2]->bst_node->sum_distances_from_zero))) {
        // Swap
        MinHeapNode* temp = heap->array[i];
        heap->array[i] = heap->array[(i - 1) / 2];
        heap->array[(i - 1) / 2] = temp;
        i = (i - 1) / 2;
    }
}

int is_in_heap(MinHeap* heap, BSTNode* bst_node) {
    for (int i = 0; i < heap->size; i++) {
        if (heap->array[i]->bst_node == bst_node) {
            return 1;  // True
        }
    }
    return 0;  // False
}

bool isInInterval(int a, int b, int c) {
    if (b == a || b == c) {
        return true;
    }

    if (a < c) {
        return b > a && b < c;
    } else {
        return b > c && b < a;
    }
}

void traverse_to_update_adjacent(BSTNode* node, BSTNode* station, MinHeap* min_heap, int dest, int src) {
    if (!node) {
        return;
    }

    int max_autonomy = get_max_autonomy_auto(station->auto_heap);

    bool ok_ricerca;
    ok_ricerca = isInInterval(dest, node->distance, src);

    if(ok_ricerca && max_autonomy != -1) {
        int distance = abs(station->distance - node->distance);
        if (distance <= max_autonomy) {
            int nuova_distance = station->distance_dijkstra + distance;
            int nuova_sum_distances_from_zero = station->sum_distances_from_zero + node->distance;

            if (nuova_distance < node->distance_dijkstra ||
                (nuova_distance == node->distance_dijkstra && nuova_sum_distances_from_zero < node->sum_distances_from_zero)) {
                node->distance_dijkstra = nuova_distance;
                node->sum_distances_from_zero = nuova_sum_distances_from_zero;
                if(node->distance != station->distance) {
                    node->prev = station;
                }

                // Se il nodo non è già nel Min Heap, inseriscilo
                if (!is_in_heap(min_heap, node)) {
                    insert_in_minHeap(min_heap, node, nuova_distance, nuova_sum_distances_from_zero);
                } else {
                    // Altrimenti, aggiorna il nodo nella coda di priorità
                    decrease_key(min_heap, node, nuova_distance);
                }
            }
        }
    }

    traverse_to_update_adjacent(node->left, station, min_heap, dest, src);
    traverse_to_update_adjacent(node->right, station, min_heap, dest, src);
}

void update_adjacent_stations(BSTNode* current_bst_node, BSTNode* root, MinHeap* min_heap, int dest, int src) {
    traverse_to_update_adjacent(root, current_bst_node, min_heap, dest, src);
}

MinHeap* create_min_heap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (heap == NULL) {
        printf("Memory allocation failed for MinHeap structure\n");
        exit(1);
    }

    heap->array = (MinHeapNode**)malloc(capacity * sizeof(MinHeapNode*));
    if (heap->array == NULL) {
        printf("Memory allocation failed for MinHeap array\n");
        free(heap);  // Libera la memoria già allocata per la struttura heap
        exit(1);
    }

    heap->size = 0;
    heap->capacity = capacity;

    return heap;
}


void dijkstra_adattato(BSTNode* root, int dest, int src) {
    if (root == NULL) return;

    // Inizializzazione
    MinHeap* min_heap = create_min_heap(MIN_HEAP_CAPACITY);  // Inizializza con una capacità arbitraria
    BSTNode* dest_node = search_station(root, dest);
    if (dest_node == NULL) return;
    dest_node->distance_dijkstra = 0;

    MinHeapNode* new_heap_node = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    new_heap_node->bst_node = dest_node;
    new_heap_node->distance_dijkstra = 0;
    min_heap->array[0] = new_heap_node;
    min_heap->size = 1;

    while (min_heap->size != 0) {
        MinHeapNode* current_heap_node = extract_min(min_heap);
        BSTNode* current_bst_node = current_heap_node->bst_node;

        // Se il nodo corrente è il nodo di arrivo, interrompi l'algoritmo.
        if (current_bst_node->distance == src) {
            free(current_heap_node);
            break;
        }

        // Per ogni stazione adiacente...
        update_adjacent_stations(current_bst_node, root, min_heap, dest, src);

        free(current_heap_node);  // Libera la memoria del nodo estratto
    }

    // Libera la memoria del min_heap
    for (int i = 0; i < min_heap->size; i++) {
        free(min_heap->array[i]);
    }
    free(min_heap->array);
    free(min_heap);
}

void stampa_percorso(BSTNode* node) {
    if (!node) return;
    printf("%d", node->distance);
    if (node->prev) {
        printf(" ");
    }
    stampa_percorso(node->prev);
}

void forward_percorso(BSTNode* node) {
    if (!node) return;
    forward_percorso(node->prev);
    if (node->prev) {
        printf(" ");
    }
    printf("%d", node->distance);
}

void pianifica_percorso(BSTNode* root, int dest, int src) {
    bool isForward = false ;
    if(src == dest)
    {
    	printf("%d \n", src);
	}
	if(dest>src)
	{
	    int temp = dest ;
	    dest = src ;
	    src = temp ;
	    isForward = true ;
	}
    reset_dijkstra_values(root);
    dijkstra_adattato(root, dest, src);
    // Ottieni il nodo di destinazione
    BSTNode* src_node = search_station(root, src);
    if (!src_node || src_node->distance_dijkstra == INT_MAX) {
        printf("nessun percorso\n");
    } else {
        if(isForward)
        {
            forward_percorso(src_node);
            printf("\n");
        }
        else
        {
            stampa_percorso(src_node);
            printf("\n");
        }
    }
}

void free_bst(BSTNode* root) {
    if (root != NULL) {
        // Libera la memoria dei sottoalberi sinistro e destro
        free_bst(root->left);
        free_bst(root->right);

        if (root->auto_heap != NULL) {
            free(root->auto_heap);
        }

        // Libera la memoria del nodo
        free(root);
    }
}

int main() {
    char comando[20];
    BSTNode* root = NULL;

    while (scanf("%s", comando) != EOF) {
        if (strcmp(comando, "aggiungi-stazione") == 0) {
            int distanza, numero_auto;


            if (scanf("%d %d", &distanza, &numero_auto) != 2) {
                printf("Errore di input\n");
                break;
            }

            int autonomie[numero_auto];

            for (int i = 0; i < numero_auto; i++) {
                if (scanf("%d", &autonomie[i]) != 1) {
                    printf("Errore di input\n");
                    break;
                }
            }

            int is_added;
            root = add_car(root, distanza, numero_auto, autonomie, &is_added);

            if (is_added) {
                printf("aggiunta\n");
            } else {
                printf("non aggiunta\n");
            }
        } else if (strcmp(comando, "demolisci-stazione") == 0) {
            int distanza;

            if (scanf("%d", &distanza) != 1) {
                printf("Errore di input\n");
                break;
            }

            int is_removed;
            root = delete_station(root, distanza, &is_removed);

            if (is_removed) {
                printf("demolita\n");
            } else {
                printf("non demolita\n");
            }
        } else if (strcmp(comando, "aggiungi-auto") == 0) {
            int distanza, autonomia;

            if (scanf("%d %d", &distanza, &autonomia) != 2) {
                printf("Errore di input\n");
                break;
            }

            int is_added;
            insert_auto(root, distanza, autonomia, &is_added);

            if (is_added) {
                printf("aggiunta\n");
            } else {
                printf("non aggiunta\n");
            }
        } else if (strcmp(comando, "rottama-auto") == 0) {
            int distanza, autonomia;

            if (scanf("%d %d", &distanza, &autonomia) != 2) {
                printf("Errore di input\n");
                break;
            }

            int is_removed;
            remove_auto(root, distanza, autonomia, &is_removed);

            if(is_removed) {
                printf("rottamata\n");
            } else {
                printf("non rottamata\n");
            }
        } else if (strcmp(comando, "pianifica-percorso") == 0) {
            int partenza, arrivo;

            if (scanf("%d %d", &partenza, &arrivo) != 2) {
                printf("Errore di input\n");
                break;
            }

            pianifica_percorso(root, arrivo, partenza);
        }
    }

    free_bst(root);

    return 0;
}

