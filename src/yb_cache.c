#include <sys/time.h>
#include <pthread.h>

#include "ybruf.h"

// Cache configuration parameters
#define CACHE_SIZE 1024
#define DISCARD_AFTER_MS 10000 /* 10 sec */

static pthread_mutex_t cache_in_use = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
  struct timeval created;  /* The time of caching */
  struct timeval accessed; /* The time of the most recent access */
  int value_len;           /* Value length */
  char *key;               /* NULL if not used */
  char *value;             /* NULL if not used */
} CacheLine;

static CacheLine cache[CACHE_SIZE];

void init_cache()
{
  // YOUR CODE HERE
  for (int i = 0; i < CACHE_SIZE; i++)
  {
    cache[i].key = NULL;
    cache[i].value = NULL;
  }
}

// Do not forget to lock and later unlock the mutex(es)!
char *cache_lookup(const char *key, int *length)
{
  // YOUR CODE HERE
  pthread_mutex_lock(&cache_in_use);

  char *return_value = NULL;
  for (int i = 0; i < CACHE_SIZE; i++)
  {
    if (cache[i].key != NULL && strcmp(cache[i].key, key) == 0)
    {
      *length = cache[i].value_len;
      // accessed timestamp is updated to current timestamp
      struct timeval current_time;
      gettimeofday(&current_time, NULL);
      cache[i].accessed.tv_sec = current_time.tv_sec;
      cache[i].accessed.tv_usec = current_time.tv_usec;

      // if the lifetime/created is longer than discard_after_ms then return null
      int cache_current_time = cache[i].accessed.tv_sec * 1000000 + cache[i].accessed.tv_usec;
      int cache_created_time = cache[i].created.tv_sec * 1000000 + cache[i].created.tv_usec;

      if (cache_current_time - cache_created_time >= DISCARD_AFTER_MS * 1000)
      {
        cache[i].key = NULL;
        cache[i].value = NULL;
        return_value = NULL;
      }
      else
      {
        return_value = malloc(cache[i].value_len);
        memcpy(return_value, cache[i].value, cache[i].value_len);
      }
    }
  }
  pthread_mutex_unlock(&cache_in_use);
  return return_value;
}

// Do not forget to lock and later unlock the mutex!
bool cache_insert(const char *key, const char *value, int length)
{
  // YOUR CODE HERE
  pthread_mutex_lock(&cache_in_use);

  char *dup_key = strdup(key);

  struct timeval current_time;
  gettimeofday(&current_time, NULL);

  int victim = 0;

  for (int i = 0; i < CACHE_SIZE; i++)
  {
    // if there's still an empty line, then i will be the victim
    //  we will insert in here
    if (cache[i].key == NULL)
    {
      victim = i; // victim: cache still have empty line
      break;
    }

    // if not then we will have to select the victim with the least accessed time
    int smallest_accessed_time = cache[0].accessed.tv_sec * 1000000 + cache[0].accessed.tv_usec;
    int current_accessed_time = cache[i].accessed.tv_sec * 1000000 + cache[i].accessed.tv_usec;
    if (current_accessed_time <= smallest_accessed_time)
    {
      smallest_accessed_time = current_accessed_time;
      victim = i; // victim: cache is full, so chose the smallest accessed time
    }
  }

  cache[victim].key = dup_key;
  cache[victim].value = (char *)value;
  cache[victim].value_len = length;
  cache[victim].accessed.tv_sec = current_time.tv_sec;
  cache[victim].accessed.tv_usec = current_time.tv_usec;
  cache[victim].created.tv_sec = current_time.tv_sec;
  cache[victim].created.tv_usec = current_time.tv_usec;

  pthread_mutex_unlock(&cache_in_use);

  return true;
}