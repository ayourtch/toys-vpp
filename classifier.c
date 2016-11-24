#include <vppinfra/pool.h>
#include <vnet/classify/vnet_classify.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>



/* Some aids in ASCII graphing the content */
#define XX "\377"
#define __ "\000"
#define _(x)
#define v

u8 ip4_5tuple_mask[] =
_("             dmac               smac            etype ")
_( ether)  __ __ __ __ __ __ v __ __ __ __ __ __ v __ __ v
_("        v ihl totlen ")
_(0x0000)  __ __ __ __
_("        ident fl+fo ")
_(0x0004)  __ __ __ __
_("       ttl pr checksum")
_(0x0008)  __ XX __ __
_("        src address    ")
_(0x000C)  XX XX XX XX
_("        dst address    ")
_(0x0010)  XX XX XX XX
_("L4 T/U  sport dport   ")
_(tcpudp)  XX XX XX XX
_(padpad)  __ __ __ __
_(padpad)  __ __ __ __
_(padeth)  __ __
;

u8 ip6_5tuple_mask[] =
_("             dmac               smac            etype ")
_( ether)  __ __ __ __ __ __ v __ __ __ __ __ __ v __ __ v
_("        v  tc + flow ")
_(0x0000)  __ __ __ __
_("        plen  nh hl  ")
_(0x0004)  __ __ XX __
_("        src address  ")
_(0x0008)  XX XX XX XX
_(0x000C)  XX XX XX XX
_(0x0010)  XX XX XX XX
_(0x0014)  XX XX XX XX
_("        dst address    ")
_(0x0018)  XX XX XX XX
_(0x001C)  XX XX XX XX
_(0x0020)  XX XX XX XX
_(0x0024)  XX XX XX XX
_("L4T/U  sport dport   ")
_(tcpudp)  XX XX XX XX
_(padpad)  __ __ __ __
_(padeth)  __ __
;

#undef XX
#undef __
#undef _
#undef v


static int
count_skip(u8 *p, u32 size)
{
  u64 *p64 = (u64 *)p;
  while ( (0ULL == *p64) && ((u8 *)p64 - p) < size ) {
    p64++;
  }
  return (p64-(u64 *)p)/2;
}


u32 nbuckets = 256;
u32 memory_size = 2<<20;
u32 opaque_index = 0;

static int
acl_classify_add_del_table(vnet_classify_main_t * cm, u8 * mask, u32 mask_len, u32 next_table_index, u32 miss_next_index, u32 *table_index, int is_add)
{
  u32 skip = count_skip(mask, mask_len);
  u32 match = (mask_len/16) - skip;
  u8 *skip_mask_ptr = mask + 16*skip;
  return vnet_classify_add_del_table(cm, skip_mask_ptr, nbuckets, memory_size, skip, match, next_table_index, miss_next_index, table_index, 0, 0, is_add);
}


void print_result(int i)
{
  printf("memory size: %u nbuckets: %u max_sessions: %u\n", memory_size, nbuckets, opaque_index);
}


int main(int argc, char *argv[])
{
  vnet_classify_main_t classify_main;
  vnet_classify_main_t *cm = &classify_main;
  u32 ip4_table_index = 0;
  int next = -1;
  int rv;
  int i;
  pid_t child_pid = 0;
  u32 session_match_next = 0;
  int status;
  int iterations = 0;

  if (argc > 1) {
    memory_size = atoi(argv[1]);
  }
 
  if (argc > 2) {
    nbuckets = atoi(argv[2]);
  }
 
  if (argc > 3) {
    iterations = atoi(argv[3]);
  }
  for(i=0; i<iterations; i++) {
    if ((child_pid = fork())) {
      wait(&status);
    } else {
      break;
    }
    memory_size = memory_size * 2;
  }
  if (child_pid) {
    printf("Parent done!\n");
    exit(0);
  }

  signal(SIGABRT, print_result);
  signal(SIGINT, print_result);
  clib_mem_init (0, 3ULL << 10);
  memset(cm, 0, sizeof(*cm));

  cm->vlib_main = vlib_get_main();
  cm->vnet_main = vnet_get_main();
  ip4_table_index = ~0;

  rv = acl_classify_add_del_table(cm, ip4_5tuple_mask, sizeof(ip4_5tuple_mask)-1, ~0, next, &ip4_table_index, 1);

  clib_warning("result of table add: %d", rv);
  while (1) {
    *(u16 *)&ip4_5tuple_mask[14 + 0x14] = 0xffff & opaque_index;
    *(u32 *)&ip4_5tuple_mask[14 + 0x0c] = opaque_index;
    rv = vnet_classify_add_del_session(cm, ip4_table_index, ip4_5tuple_mask, session_match_next, opaque_index++, 0, 42, ~0, 1);
    if (rv) {
      printf("Non-zero rv: %d, opaque_index: %d\n", rv, opaque_index);
    }
  }
}
