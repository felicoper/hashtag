#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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


int main(){
	printf("%zu\n",jenkins_one_at_a_time_hash("x",30));
  	printf("%zu\n",jenkins_one_at_a_time_hash("mini cooper",30));
    	printf("%zu\n",jenkins_one_at_a_time_hash("regex",30));
      	printf("%zu\n",jenkins_one_at_a_time_hash("admin",30));
  return 0;
}
