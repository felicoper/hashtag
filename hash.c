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
	nodo_t** tabla;
	size_t cant;
	size_t cap;
	hash_destruir_dato_t destruir_dato;
};

struct hash_iter{
	const hash_t* hash;
	size_t pos;
};

//ni idea como - pero funciona
size_t jenkins_one_at_a_time_hash(const char* clave, size_t hash_cap) {
  size_t i = 0;
  size_t hash = 0;
  size_t length = strlen(key);
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

	hash->tabla = malloc(sizeof(void*)*TAM_HASH);

	if(!hash->tabla){
		free(hash);
		return NULL;
	}
	hash->cant = 0;
	hash->cap = TAM_HASH;
	hash->destruir_dato = destruir_dato;

	for(int i=0;i<TAM_HASH;i++){
		hash->tabla[i]=crear_nodo();
		if(!hash->tabla[i]) return NULL;
	}
	return hash;
}

bool hash_guardar(hash_t* hash, const char* clave, void *dato){
	char* clave_guardar = malloc(sizeof(char)*strlen(clave));
	if (!clave_guardar) return false;

	//Escupe el indice entre 0 y tam_hash
	size_t pos=jenkins_one_at_a_time_hash(clave,hash->hash_cap);
	while(pos<hash->cap){
		if(hash->tabla[pos]->estado==VACIO || hash->tabla[pos]->estado==BORRADO){
				hash->tabla[pos]->key = clave_guardar;
				hash->tabla[pos]->estado = OCUPADO;
				hash->tabla[pos]->dato = dato;

				hash->cant ++;
		}
		else{
			pos++;
			if (pos+1==hash_cap) pos=0;
		}
	}
	/*if (factor_carga >= 0.8) redimensionar(hash);*/
	return true;
}

void* hash_borrar(hash_t* hash,const char* clave){
	size_t pos = jenkins_one_at_a_time_hash(clave,hash->cap);
	void* dato_aux = NULL;
	int i=0;
	while(pos<hash->cap){
		if(i==hash->cap) return NULL; //no encontro el elemento y loopeo todo el hash
		if(hash->tabla[pos]->estado==OCUPADO && hash->tabla[pos]->key==clave){
			dato_aux = hash->tabla[pos]->dato;
			hash->tabla[pos]->dato = NULL;
			free(hash->tabla[pos]->key);
			hash->cant --;
		}
		else{
		pos++;
		if(pos+1==hash->cap) pos=0;
		i++;
		}
		if
		dato_aux = hash->tabla[pos]->dato;
		hash->tabla[pos]->dato = NULL;
		free(hash->tabla[pos]->key);
		hash->cant --;
	}
	/* hacer redimension if factor_carga < 0.3 */
	return dato_aux;
}

/*

MODULARIZAR BUSQUEDA:

Que devuelve buscar_campo? int ? esta mal despues castearlo a size_t?
si no encuentra el campo devuelve -1, si devuelve -1 el campo no esta
si el campo no esta devuelve null


size_t buscar_campo(const hash_t *hash,const char* clave){
	size_t pos_campo = jenkins_one_at_a_time_hash(clave,hash->cap);
	int i=0;
	while(pos<hash->cap){
		if(i==hash->cap) return NULL; //no encontro el elemento y loopeo todo el hash
		if(hash->tabla[pos_campo]->estado==OCUPADO && hash->tabla[pos_campo]->key==clave){
			return pos_campo;
			}
		else{
		pos++;
		if(pos+1==hash->cap) pos=0;
		i++;
		}
	}
	return false;
}

*/

void hash_obtener(const hash_t *hash, const char *clave){
	size_t pos = jenkins_one_at_a_time_hash(clave,hash->cap);
	void* valor = NULL;
	while(pos<hash->cap){
		if(i==hash->cap) return NULL; //no encontro el elemento y loopeo todo el hash
		if(hash->tabla[pos]->estado==OCUPADO && hash->tabla[pos]->key==clave){
			valor = hash->tabla[pos]->dato;
			}
		else{
		pos++;
		if(pos+1==hash->cap) pos=0;
		i++;
		}
	}
	return valor;
}


size_t hash_cantidad(const hash_t *hash){
	if(!hash) return NULL;
	return hash->cant;
}
