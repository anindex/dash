/*
 * dart_tasking_datadeps.h
 *
 *  Created on: Nov 8, 2016
 *      Author: joseph
 */

#ifndef DART_TASKING_DATADEPS_H_
#define DART_TASKING_DATADEPS_H_

#include <dash/dart/tasking/dart_tasking_priv.h>

typedef void* remote_task_t;

typedef union taskref {
  dart_task_t         *local;
  remote_task_t        remote;
} taskref;

/**
 * Initialize the data dependency management system.
 */
dart_ret_t dart_tasking_datadeps_init() DART_INTERNAL;

dart_ret_t dart_tasking_datadeps_reset(dart_task_t *task) DART_INTERNAL;

dart_ret_t dart_tasking_datadeps_fini() DART_INTERNAL;

/**
 * Search for tasks that satisfy the data dependencies of this task.
 */
dart_ret_t dart_tasking_datadeps_handle_task(
    dart_task_t           *task,
    const dart_task_dep_t *deps,
    size_t                 ndeps) DART_INTERNAL;

/**
 * Look for the latest task that satisfies \c dep and add \rtask
 * to the remote successor list.
 * Note that \c dep has to be a IN dependency.
 */
dart_ret_t dart_tasking_datadeps_handle_remote_task(
    const dart_task_dep_t  *dep,
    const taskref           remote_task,
    dart_global_unit_t      origin) DART_INTERNAL;

/**
 * Handle the direct task dependency between a local task
 * and it's remote successor
 */
dart_ret_t dart_tasking_datadeps_handle_remote_direct(
    dart_task_t        *local_task,
    taskref             remote_task,
    dart_global_unit_t  origin) DART_INTERNAL;

/**
 * End a phase, e.g., by releasing any unhandled remote dependency of the
 * same phase.
 */
dart_ret_t dart_tasking_datadeps_end_phase(uint64_t phase) DART_INTERNAL;

/**
 * Release the dependencies of \c task, potentially enqueuing t
 * asks into the runnable queue of \c thread.
 */
dart_ret_t
dart_tasking_datadeps_release_local_task(dart_task_t   *task) DART_INTERNAL;

/**
 * Release a remote dependency of the \c local_task.
 * Might defer the release if the task is not eligible to run, yet.
 */
dart_ret_t dart_tasking_datadeps_release_remote_dep(
    dart_task_t *local_task) DART_INTERNAL;

/**
 * Cancel all remaining remote dependencies.
 * All tasks that are still blocked by remote dependencies will be subsequently
 * released if they have no local dependecies.
 */
dart_ret_t dart_tasking_datadeps_cancel_remote_deps() DART_INTERNAL;

/**
 * Release all unhandled remote dependency requests.
 * This should be done before starting to execute local tasks
 * to avoid deadlocks.
 */
dart_ret_t
dart_tasking_datadeps_release_unhandled_remote() DART_INTERNAL;

DART_INLINE
bool
dart_tasking_datadeps_is_runnable(dart_task_t *task) {
  return (task->unresolved_deps == 0) && (task->unresolved_remote_deps == 0);
}


DART_INLINE
dart_gptr_t
dart_tasking_datadeps_localize_gptr(dart_gptr_t gptr)
{
  void *addr;
  dart_gptr_getaddr(gptr, &addr);
  DART_ASSERT(addr != NULL);
  dart_gptr_t res = gptr;
  res.addr_or_offs.addr = addr;
  res.segid = 0;
  return res;
}


/**
 * Check for new remote task dependency requests coming in
 */
dart_ret_t dart_tasking_datadeps_progress() DART_INTERNAL;

#endif /* DART_TASKING_DATADEPS_H_ */