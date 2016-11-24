#include <vppinfra/pool.h>

typedef struct {
 u32 foo;
} acl_list_t;

int main (int argc, char *argv[])
{
  int i;
  acl_list_t *testpool = 0, *a, *b, *c;
  clib_mem_init (0, 3ULL << 10);
  clib_warning("init");

  pool_get_aligned (testpool, a, CLIB_CACHE_LINE_BYTES);
  clib_warning("Pool indices: %ld\n", a - testpool);
  pool_get_aligned (testpool, b, CLIB_CACHE_LINE_BYTES);
  clib_warning("Pool indices: %ld\n", b - testpool);
  pool_get_aligned (testpool, c, CLIB_CACHE_LINE_BYTES);
  clib_warning("Pool indices: %ld\n", c - testpool);
}

