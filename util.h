#ifndef UTIL_H_
#define UTIL_H_

void
safe_mutex_lock(pthread_mutex_t *mutex);

void
safe_mutex_unlock(pthread_mutex_t *mutex);

int32_t
detect_cpus();

#endif
