#include "hash.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#define TAM_HASH 30


typedef enum{VACIO,OCUPADO,BORRADO}estado_t;

typedef struct nodo{
	const char* clave;
	void* dato;
	estado_t estado;
} nodo_t;

struct hash{
	nodo_t** tabla;
	size_t cant;
	size_t cap;
	hash_destruir_dato_t destruir_dato;
};

struct hash_iter{
	const hash_t* hash;
	size_t pos;
};

size_t jenkins_one_at_a_time_hash(const char* clave, size_t hash_cap) {
  size_t i = 0;
  size_t hash = 0;
  size_t length = strlen(clave);
  while (i != length) {
    hash += clave[i++];
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
	nodo->dato = NULL;
	nodo->clave = NULL;
	nodo->estado = VACIO;
	return nodo;
}

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
	hash_t* hash = malloc(sizeof(hash_t));
	if(!hash) return NULL;

	hash->tabla = malloc(sizeof(nodo_t)*TAM_HASH);
	if(!hash->tabla){
		free(hash);
		return NULL;
	}
	hash->cap = TAM_HASH;
	for(size_t i=0;i<hash->cap;i++){
		nodo_t* nodo = crear_nodo();
		if(!nodo) return NULL;
		hash->tabla[i]=nodo;
	}
	hash->cant = 0;
	hash->destruir_dato = destruir_dato;

	return hash;
}

bool hash_guardar(hash_t* hash, const char* clave, void *dato){
	char* clave_aux = malloc(sizeof(char)*strlen(clave));
	if (!clave_aux) return false;
	size_t pos=jenkins_one_at_a_time_hash(clave,hash->cap);
	size_t i=0;
	//me fijo si la clave existe
	for(;i<hash->cap;i++){		//o(N)
		if(hash->tabla[i]->estado==OCUPADO && hash->tabla[i]->clave == clave_aux){
			hash->destruir_dato(hash->tabla[i]->dato);
			hash->tabla[i]->dato = dato;
			return true;
		}
	}
	//si no busco el dato
	while(pos<hash->cap){
		if(hash->tabla[pos]->estado==VACIO){
			hash->tabla[pos]->clave = clave_aux;
			hash->tabla[pos]->dato = dato;
			hash->tabla[pos]->estado = OCUPADO;
			hash->cant ++;
		}
		pos++;
		if(pos+1==hash->cap) pos=0;
	}
	return true;
}

void *hash_borrar(hash_t *hash, const char *clave){
	//mirar redimension
	if(hash->cant==0) return NULL;
	size_t pos=jenkins_one_at_a_time_hash(clave,hash->cap);
	void* dato = NULL;
	//busco la clave y me fijo si existe
	int i=0;
	while(pos<hash->cap){
		if(i==hash->cap) return dato;
		if(hash->tabla[pos]->estado==OCUPADO && hash->tabla[pos]->clave==clave){
			dato = hash->tabla[pos]->dato;
			free((char*)hash->tabla[pos]->clave);
			hash->tabla[pos]->estado=BORRADO;
			hash->cant--;
		}
		else{
			pos++,i++;
		}
	}
	return dato;
}


void *hash_obtener(const hash_t *hash, const char *clave){
	if(hash->cant==0) return NULL;
	void* dato = NULL;
	size_t pos = jenkins_one_at_a_time_hash(clave,hash->cap);
	size_t i=0;
	while(i<hash->cap){
		if(hash->tabla[pos]->estado==VACIO || i+1==hash->cap) return dato;
		if(hash->tabla[pos]->clave == clave){
			dato = hash->tabla[pos]->dato;
			break;
		}
		pos++;
	}
	return dato;
}

bool hash_pertenece(const hash_t *hash,const char *clave){
	return (hash_obtener(hash,clave)!=NULL);
}

size_t hash_cantidad(const hash_t* hash){
	return hash->cant;
}

void hash_destruir(hash_t* hash){
	if(hash->cant != 0){
		for(size_t i=0;i<hash->cap;i++){
			if(hash->tabla[i]->estado==OCUPADO){
				hash->destruir_dato((char*)hash->tabla[i]->clave);
				hash->destruir_dato(hash->tabla[i]->dato);
			}
		}
	}
	free(hash->tabla);
	free(hash);
}

hash_iter_t *hash_iter_crear(const hash_t *hash){
	hash_iter_t* iter = malloc(sizeof(hash_iter_t));
	if(!iter) return NULL;
	iter->hash = hash;
	//busco el primer ocupado
	if(iter->hash->cant==0){
		iter->pos = hash->cap;
		return iter;
	}
	for(size_t i=0;i<iter->hash->cap;i++){
		if(iter->hash->tabla[i]->estado==OCUPADO){
			iter->pos = i;
		}
	}
	return iter;
}

bool hash_iter_al_final(const hash_iter_t *iter){
	return (iter->pos==iter->hash->cap);

}

bool hash_iter_avanzar(hash_iter_t *iter){
	if(!hash_iter_al_final(iter)){
		iter->pos+=1;
		return true;
	}
	return false;
}

const char* hash_iter_ver_actual(const hash_iter_t* iter){
	if(hash_iter_al_final(iter)) return NULL;
	return iter->hash->tabla[iter->pos]->clave;
}

void hash_iter_destruir(hash_iter_t* iter){
	free(iter);
}
