#include <sys/time.h>
#include <pthread.h>

#include "ybruf.h"

// Cache configuration parameters
#define CACHE_SIZE 1024
#define DISCARD_AFTER_MS 10000 /* 10 sec */

// A useful macro
#define TV_TO_USEC(tv) (tv.tv_sec * 1000000 + tv.tv_usec)

static pthread_mutex_t cache_in_use = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
  struct timeval created;	/* The time of caching */
  struct timeval accessed;	/* The time of the most recent access */
  int value_len;		/* Value length */
  char *key;			/* NULL if not used */
  char *value;			/* NULL if not used */
} CacheLine;

static CacheLine cache[CACHE_SIZE];

void init_cache()
{
  for(int i = 0; i < CACHE_SIZE; i++)
    cache[i].key = cache[i].value = NULL;
}

// Do not forget to lock and later unlock the mutex(es)!
char *cache_lookup(const char *key, int *length)
{
  char *value = NULL;

  // Secure the cache
  pthread_mutex_lock(&cache_in_use);
  
  for(int i = 0; i < CACHE_SIZE; i++) {
    // The right key
    if(cache[i].key && !strcmp(cache[i].key, key)) {
      struct timeval now;

      // Is the line "fresh" or stale?
      gettimeofday(&now, NULL);
      if(TV_TO_USEC(now) - TV_TO_USEC(cache[i].created)
	 > DISCARD_AFTER_MS * 1000) {
	// Discard
	free(cache[i].value);
	cache[i].key = cache[i].value = NULL;
      } else {
	// Duplicate
	value = malloc(cache[i].value_len);
	if (value) {
	  memcpy(value, cache[i].value, cache[i].value_len);
	  *length = cache[i].value_len;
	  cache[i].accessed = now;
	}
      }
      break;
    }
  }

  // Release the cache
  pthread_mutex_unlock(&cache_in_use);
  return value;
}

// Do not forget to lock and later unlock the mutex!
bool cache_insert(char *key, const char *value, int length)
{
  // Secure the cache
  pthread_mutex_lock(&cache_in_use);

  // Find the oldest and/or an empty line
  int oldest_time = TV_TO_USEC(cache[0].accessed);
  int oldest_position = -1;  
  int i;
  for(i = 0; i < CACHE_SIZE && cache[i].key; i++) {
    int current_time = TV_TO_USEC(cache[i].accessed);
    if (current_time < oldest_time) {
      oldest_time = current_time;
      oldest_position = i;
    }
  }
  
  // All lines used, select the victim 
  if (i == CACHE_SIZE) {	
    i = oldest_position;
    free(cache[i].value);
    cache[i].key = cache[i].value = NULL;
  }

  // Duplicate the value
  cache[i].key = key;
  cache[i].value = malloc(length);
  if (!cache[i].value) {
    pthread_mutex_unlock(&cache_in_use);
    return false;
  }
  memcpy(cache[i].value, value, length);
  
  // Release the cache
  pthread_mutex_unlock(&cache_in_use);
  return true;
}
