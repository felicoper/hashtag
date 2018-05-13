#include "hash.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#define TAM_HASH 30
#define COEF_REDIM 2
#define CANT_MAX 3/4
#define CANT_MIN 1/4

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

bool hash_esta_vacio(const hash_t* hash){
	return hash->cant==0;
}

nodo_t* crear_nodo(){
	nodo_t* nodo = malloc(sizeof(nodo_t));
	if(!nodo) return NULL;
	nodo->dato = NULL;
	nodo->clave = NULL;
	nodo->estado = VACIO;
	return nodo;
}

bool resize_hash(hash_t* hash,size_t nuevo_tam){
	//creo tabla nueva 
	nodo_t** nueva_tabla=malloc(sizeof(nodo_t)*nuevo_tam);
	if(!nueva_tabla)	return false;
	for(size_t i=0;i<nuevo_tam;i++){
		nodo_t* nodo=crear_nodo();
		if(!nodo) return false;
		nueva_tabla[i]=nodo;
	}

	//recorro el hash viejo buscando solo los ocupados y los rehasheo a la tabla nueva
	for(size_t i=0;i<hash->cap;i++){
		if(hash->tabla[i]->estado==OCUPADO){
			size_t pos = jenkins_one_at_a_time_hash(hash->tabla[i]->clave,nuevo_tam);
			//me fijo si la nueva posicion esta ocupada o no y guardo el elemento;
			while(true){
				if(pos+1==nuevo_tam) pos=0;
				if(nueva_tabla[pos]->estado==VACIO){
					nueva_tabla[pos]->estado=OCUPADO;
					nueva_tabla[pos]->clave=strdup(hash->tabla[i]->clave);
					nueva_tabla[pos]->dato=hash->tabla[i]->dato;
					break;
				}
				else{
					pos++;
				}
			}
		}
			free((char*)hash->tabla[i]->clave);
		free(hash->tabla[i]);
	}
	nodo_t** tabla_previa = hash->tabla;
	hash->tabla = nueva_tabla;
	hash->cap=nuevo_tam;
	free(tabla_previa);
	return true;
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
	//Si la cantidad esta en 3/4 del total que se duplique el tamanio
	if(hash->cant >= hash->cap*CANT_MAX) resize_hash(hash,hash->cap*COEF_REDIM);

	size_t pos=jenkins_one_at_a_time_hash(clave,hash->cap);
	//me fijo si la clave existe
	if(hash_pertenece(hash,clave)){
		if(hash->destruir_dato) hash->destruir_dato(hash->tabla[pos]->dato);
		hash->tabla[pos]->dato=dato;
	}
	else{
		size_t i=0;
		while(i<hash->cap){
			if(pos+1==hash->cap) pos=0;
			if(hash->tabla[pos]->estado!=OCUPADO){
				hash->tabla[pos]->estado=OCUPADO;
				hash->tabla[pos]->clave=strdup(clave);
				hash->tabla[pos]->dato=dato;
				hash->cant++;
				break;
			}
			else{
				i++,pos++;
			}
		}

	}
	return true;
}


void *hash_borrar(hash_t *hash, const char *clave){
	//si la cantidad de elementos del hash es < a 1/4 de su tamanio que redimensione a la mitad
	if(hash->cant < hash->cap*CANT_MIN){
		if(!resize_hash(hash,hash->cap/COEF_REDIM)) return NULL;
	}


	if(hash_esta_vacio(hash)) return NULL;
	size_t pos=jenkins_one_at_a_time_hash(clave,hash->cap);
	void* dato = NULL;
	//busco la clave y me fijo si existe
	size_t i=0;
	while(hash->tabla[pos+i]->estado==OCUPADO && i<hash->cap){
		
		if(strcmp(hash->tabla[pos+i]->clave,clave)==0){
			dato=hash->tabla[pos]->dato;
			hash->tabla[pos]->dato=NULL;
			hash->tabla[pos]->estado=BORRADO;
			hash->cant--;
			break;
		}
		else{
			pos++,i++;
			if(pos+1==hash->cap) pos=0;
		}
	}
	return dato;
}


void *hash_obtener(const hash_t *hash, const char *clave){
	if(hash_esta_vacio(hash)) return NULL;
	void* dato=NULL;
	size_t pos = jenkins_one_at_a_time_hash(clave,hash->cap);
	for(size_t i=0;i<hash->cap;i++){
		if(pos+1==hash->cap) pos=0;
		if(hash->tabla[pos]->estado==OCUPADO && strcmp(hash->tabla[pos]->clave,clave)==0){
			return dato=hash->tabla[pos]->dato;
		}
		pos++;

	}
	return NULL;
}

bool hash_pertenece(const hash_t *hash,const char *clave){
	bool encontro=false;
	if(hash_esta_vacio(hash)) return encontro;
	size_t pos = jenkins_one_at_a_time_hash(clave,hash->cap);
	for(size_t i=0;i<hash->cap;i++){
		if(pos+1==hash->cap) pos=0;
		if(hash->tabla[pos]->estado==OCUPADO && strcmp(hash->tabla[pos]->clave,clave)==0){
			return encontro = true;
		}
	}
	return encontro;
}

size_t hash_cantidad(const hash_t* hash){
	return hash->cant;
}

void hash_destruir(hash_t* hash){
	for(size_t i=0;i<hash->cap;i++){
		if(hash->destruir_dato){
			if(hash->tabla[i]->estado==OCUPADO){
				free((char*)hash->tabla[i]->clave);
				hash->destruir_dato(hash->tabla[i]->dato);
			}
		}
		free(hash->tabla[i]);
	}
	free(hash->tabla);
	free(hash);
}


hash_iter_t *hash_iter_crear(const hash_t *hash){
	hash_iter_t* iter = malloc(sizeof(hash_iter_t));
	if(!iter) return NULL;
	iter->hash = hash;
	//hash vacio
	size_t i=0;
	if(iter->hash->cant==0){
		iter->pos = hash->cap;
	}
	while(i!=iter->hash->cap)
		if(iter->hash->tabla[i]->estado==OCUPADO){
			iter->pos=i;
			break;
		}
		else{
			i++;
		}
	return iter;
}


bool hash_iter_al_final(const hash_iter_t *iter){
	return (iter->pos==iter->hash->cap);
}

bool hash_iter_avanzar(hash_iter_t *iter){
	while(!hash_iter_al_final(iter)){
		if(iter->hash->tabla[iter->pos++]->estado==OCUPADO) return true;
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