#ifndef DART_PMEM_H_INCLUDED
#define DART_PMEM_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#include <dash/dart/if/dart_types.h>
#include <dash/dart/if/dart_globmem.h>


#define DART_INTERFACE_ON

/**
 * \file dart_pmem.h
 *
 * Routines for allocation and reclamation of persistent memory regions
 * in global address space
 */

/**
 * \defgroup  DartPersistentMemory    Persistent memory semantics
 * \ingroup   DartInterface
 */


struct dart_pmem_pool;
typedef struct dart_pmem_pool dart_pmem_pool_t;

struct dart_pmem_oid;
typedef struct dart_pmem_oid  dart_pmem_oid_t;

struct dart_pmem_pool_stat {
  size_t num_buckets;
  size_t total;
};

#define DART_PMEM_FILE_CREATE (1 << 0)
#define DART_PMEM_FILE_EXCL   (1 << 1)

/* ======================================================================== *
 * Open and Close                                                           *
 * ======================================================================== */

dart_ret_t dart__pmem__init(void);
dart_ret_t dart__pmem__finalize(void);

/* ======================================================================== *
 * Open and Close                                                           *
 * ======================================================================== */

dart_pmem_pool_t * dart__pmem__open(
  dart_team_t         team,
  char const     *    name,
  int                 flags,
  mode_t              mode);

dart_ret_t dart__pmem__close(
  dart_pmem_pool_t ** pool);

/* ======================================================================== *
 * Persistent Memory Allocation                                             *
 * ======================================================================== */

dart_pmem_oid_t dart__pmem__alloc(
  dart_pmem_pool_t  const * pool,
  size_t              nbytes);

dart_ret_t  dart__pmem__free(
  dart_pmem_oid_t    *    poid);

dart_ret_t dart__pmem__getaddr(
  dart_pmem_oid_t oid,
  void ** addr);

dart_ret_t dart__pmem__persist(
  dart_pmem_pool_t * pool,
  void * addr,
  size_t nbytes);

dart_ret_t dart__pmem__fetch_all(
    dart_pmem_pool_t * pool,
    dart_pmem_oid_t * buf
);

/* ======================================================================== *
 * Other functions                                                          *
 * ======================================================================== */

dart_ret_t dart__pmem__pool_stat(
  dart_pmem_pool_t * pool,
  struct dart_pmem_pool_stat * stat
);

dart_ret_t dart__pmem__oid_size(
  dart_pmem_pool_t const * pool,
  dart_pmem_oid_t oid,
  size_t * size
);

//TODO: move to dart base
//
/* ======================================================================== *
 * Implementation Details                                                  *
 * ======================================================================== */

#include <libpmemobj.h>
#include "pmem_list.h"

#define DART_NVM_POOL_NAME (1024)
#define DART_PMEM_MIN_POOL PMEMOBJ_MIN_POOL

struct dart_pmem_pool {
  size_t          poolsize;
  dart_team_t     teamid;
  char const   *  path;
  char const   *  layout;
  PMEMobjpool  *  pop;
};

struct dart_pmem_oid {
  PMEMoid     oid;
};

static struct dart_pmem_oid const DART_PMEM_OID_NULL = {{0, 0}};

struct dart_pmem_list_constr_args {
  char const * name;
  size_t team_size;
};

struct dart_pmem_bucket_alloc_args {
  //size_t element_size;
  size_t nbytes;
};


POBJ_LAYOUT_BEGIN(dart_pmem_bucket_list);

//list_root
POBJ_LAYOUT_ROOT(dart_pmem_bucket_list, struct dart_pmem_bucket_list)
//list element
POBJ_LAYOUT_TOID(dart_pmem_bucket_list, struct dart_pmem_bucket)
//
POBJ_LAYOUT_END(dart_pmem_bucket_list)

#ifndef DART_PMEM_TYPES_OFFSET
#define DART_PMEM_TYPES_OFFSET 2183
#endif

//define type num for void elements
TOID_DECLARE(char, DART_PMEM_TYPES_OFFSET + 0);
#define TYPE_NUM_BYTE (TOID_TYPE_NUM(char))

#ifndef MAX_BUFFLEN
#define MAX_BUFFLEN 30
#endif

struct dart_pmem_bucket_list {
  //name of pmem pool
  char                name[MAX_BUFFLEN];
  //Team size
  size_t              team_size;
  //Number of allocated buckets
  size_t              num_buckets;
  //Number of bytes for a single element
  //size_t              element_size;
  //Head node to the first bucket
  DART_PMEM_TAILQ_HEAD(dart_pmem_list_head, struct dart_pmem_bucket) head;
};

struct dart_pmem_bucket {
  //Number of elements in this bucket
  size_t     nbytes;
  //Persistent Data Buffer
  PMEMoid    data;
  //Pointer to next bucket
  DART_PMEM_TAILQ_ENTRY(struct dart_pmem_bucket) next;
};

#define DART_INTERFACE_OFF

#ifdef __cplusplus
}
#endif

#endif /* DART_PMEM_H_INCLUDED */
