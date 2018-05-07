#include "hash.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define TAM_HASH 30

//Cerrado

typedef enum{VACIO,OCUPADO,BORRADO}estado_t;

typedef struct nodo{
	const char* key;
	void* dato;
	estado_t estado;
} nodo_t;

struct hash{
	nodo_t** datos;
	size_t cant;
	size_t cap;
	hash_destruir_dato_t destruir_dato;
};

struct hash_iter{
	const hash_t* hash;
	size_t pos;
};

//ni idea como - pero funciona 
size_t jenkins_one_at_a_time_hash(const char* key, size_t hash_cap) {
  size_t i = 0;
  size_t hash = 0;
  size_t length = strlen(key);
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash % hash_cap;
}

nodo_t* crear_nodo(){
	nodo_t* nodo = malloc(sizeof(nodo_t));
	if(!nodo) return NULL;
	nodo->key = malloc(sizeof(char*));
	nodo->dato = malloc(sizeof(void*));
	if(!nodo->key || !nodo->dato){
		free(nodo);
		return NULL;
	}
	nodo->estado = VACIO;
	return nodo;
}

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
	hash_t* hash = malloc(sizeof(hash_t));
	if(!hash) return NULL;

	hash->datos = malloc(sizeof(void*)*TAM_HASH);

	if(!hash->datos){
		free(hash);
		return NULL;
	}
	hash->cant = 0;
	hash->cap = TAM_HASH;
	hash->destruir_dato = destruir_dato;

	for(int i=0;i<TAM_HASH;i++){
		hash->datos[i]=crear_nodo();
		if(!hash->datos[i]) return NULL;
	}

	return hash;
}


bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	char key = clave;
	size_t pos = jenkins_one_at_a_time_hash(clave,hash->cap);

	int i=0;
	while(i < hash->cap){
		if(hash->datos[pos]->estado==VACIO){
			hash->datos[pos]->estado=OCUPADO;
			hash->datos[pos]->key=key;
			hash->datos[pos]->dato=dato;
		}
		else pos++;
		i++;
		if(i==hash->cap){
			i=0;
	}

	hash->cant ++;
	return true;
}
