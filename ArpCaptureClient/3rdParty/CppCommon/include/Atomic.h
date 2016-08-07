#ifndef ___ATOMIC_OPS_H_
#define ___ATOMIC_OPS_H_

#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>

#define ll_low(x)       *(((unsigned long*)&(x))+0)
#define ll_high(x)      *(((unsigned long*)&(x))+1)

// Compare and Swap 32-bit
//
//   tgt = addr of value to update
//   old = addr of a value that we think is equal to the current value
//   rep = value we want to replace the current value if *old == current
//
// Atomicly perform the following:
//
//   if(*tgt == *old)
//   {
//       *tgt = rep;
//       return true;
//   }
//   else
//   {
//       *old = *tgt;
//       return false;
//   }
//
#if defined (__x86_64__) || defined (__amd64__)
static inline int atomic_cas32(volatile unsigned long* tgt,
                                   unsigned long* old,
                                   unsigned long rep)
{
    char res;

    __asm __volatile(
    "lock;	cmpxchgq %2,%1 ;"
    "       sete	%0 ;	"
    : "=a" (res),
      "=m" (*tgt)
    : "r" (rep),
      "a" (*old),
      "m" (*tgt)
    : "memory");

    return (res);
}
#else
static inline int atomic_cas32(volatile unsigned long* tgt,
                                   unsigned long* old,
                                   unsigned long rep)
{
    unsigned char rc = 0;

    __asm__ __volatile__(
        "       movl            %1, %%eax\n"
        "       movl            %3, %%edx\n"
        "lock;  cmpxchgl        %%edx, %0\n"
        "       movl            %%eax, %1\n"
        "       sete            %2\n"
        : "+o" (*tgt), "=m" (*old), "=m" (rc)
        : "m"  (rep)
        : "memory", "eax", "edx", "cc");

    return (rc);
}
#endif
// Compare and Swap 64-bit
//
//   tgt = addr of value to update
//   old = addr of a value that we think is equal to the current value
//   rep = value we want to replace the current value if *old == current
//
// Atomicly perform the following:
//
//   if(*tgt == *old)
//   {
//       *tgt = rep;
//       return true;
//   }
//   else
//   {
//       *old = *tgt;
//       return false;
//   }
//
static inline int atomic_cas64(volatile unsigned long long* tgt,
                                   unsigned long long* old,
                                   unsigned long long rep)
{
    unsigned char rc = 0;

    __asm__ __volatile__(
        "       pushl       %%ebx\n"
        "       movl        %1, %%edi\n"
        "       movl        %2, %%esi\n"
        "       movl        0(%%esi), %%eax\n"
        "       movl        4(%%esi), %%edx\n"
        "       movl        %3, %%ebx\n"
        "       movl        %4, %%ecx\n"
        "lock;  cmpxchg8b   0(%%edi)\n"
        "       movl        %%eax, 0(%%esi)\n"
        "       movl        %%edx, 4(%%esi)\n"
        "       sete        %0\n"
        "       popl        %%ebx\n"
        : "=m" (rc)
        : "m" (tgt), "m" (old), "m" (ll_low(rep)), "m" (ll_high(rep))
        : "memory", "eax", "ecx", "edx", "esi", "edi", "cc");

    return (rc);
}

// Compare and Swap 128-bit
//
//   mem = addr of value to update
//   old = a value that we think is equal to the current value
//   rep = value we want to replace the current value if old == current
//
// Atomicly perform the following:
//
//   if(*mem == old)
//   {
//       *mem = rep;
//       return true;
//   }
//   else
//   {
//       return false;
//   }
//
struct atom128_t
{
    uint64_t lo;
    uint64_t hi;
}__attribute__ (( __aligned__( 16 ) ));

static inline char atomic_cas128(volatile struct atom128_t *mem,
                                    struct atom128_t old,
                                    struct atom128_t rep)
{

    unsigned long old_h = old.hi, old_l = old.lo;
    unsigned long rep_h = rep.hi, rep_l = rep.lo;

    char r = 0;
    __asm__ __volatile__("lock; cmpxchg16b (%6);"
                 "setz %7; "
                 : "=a" (old_l),
                   "=d" (old_h)
                 : "0" (old_l),
                   "1" (old_h),
                   "b" (rep_l),
                   "c" (rep_h),
                   "r" (mem),
                   "m" (r)
                 : "cc", "memory");
    return r;
}

static inline int cas_lock(volatile unsigned long *value)
{
    unsigned long tid = (unsigned long)pthread_self();
    if (*value == tid)
        return 0;

    while(1)
    {
        unsigned long old = 0;
        atomic_cas32(value, &old, tid);
        if (*value == tid)
            return 0;
        sleep(0);
    }

    return -1;
}

static inline int cas_unlock(volatile unsigned long *value)
{
    *value = 0;
    return 0;
}

/*
 * CTM atomic struct
 */
typedef struct {
    unsigned long v;
} atomic_t;

static inline void atomic_set(atomic_t *ca, unsigned long v)
{
    ca->v = v;
}

static inline unsigned long atomic_get(atomic_t *ca)
{
    return ca->v;
}

static inline void atomic_inc(atomic_t *ca)
{
    asm volatile ("lock; incq %0"::"m"(ca->v):"memory");
}

static inline void atomic_dec(atomic_t *ca)
{
    asm volatile ("lock; decq %0"::"m"(ca->v):"memory");
}

static inline int atomic_inc_test(atomic_t *ca)
{
    unsigned char c;
    asm volatile ("lock;incq %1; setz %0" :"=qm"(c) :"m"(ca->v) :"memory");
    return c != 0;
}

static inline int atomic_dec_test(atomic_t *ca)
{
    unsigned char c;
    asm volatile ("lock;decq %1; setz %0" :"=qm"(c) :"m"(ca->v) :"memory");
    return c != 0;
}

#endif

