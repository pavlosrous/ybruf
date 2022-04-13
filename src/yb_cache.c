#include <sys/time.h>
#include <pthread.h>

#include "ybruf.h"

// Cache configuration parameters
#define CACHE_SIZE 1024
#define DISCARD_AFTER_MS 10000 /* 10 sec */

static pthread_mutex_t cache_in_use = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
  struct timeval created;	/* The time of caching */
  struct timeval accessed;	/* The time of the most recent access */
  char *key;			/* NULL if not used */
  char *value;			/* NULL if not used */
} CacheLine;

static CacheLine cache[CACHE_SIZE];

void init_cache()
{
  // YOUR CODE HERE
}

// Do not forget to lock and later unlock the mutex(es)!
char *cache_lookup(const char *key)
{
  // YOUR CODE HERE
  return NULL;
}

// Do not forget to lock and later unlock the mutex!
bool cache_insert(const char *key, const char *value)
{
  // YOUR CODE HERE
  return true;
}
