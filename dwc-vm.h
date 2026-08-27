// A Tachyon inspired system, MIT license, (c) 2026 Chris Curl

#ifndef __DWC_VM_H__
#define __DWC_VM_H__
#include <stdint.h>
#include "kernel.h"

#define VERSION         20260831

#define LIT_MASK      0x40000000
#define LIT_BITS      0x3FFFFFFF
#define CELL_SZ                4
#define NAME_SZ               26
#define cell             int32_t
#define ucell           uint32_t
#define NULL                   0

#define byte             uint8_t
#define MEM_SZ         16*1024*1024  /* 16MB */
#define STK_SZ                63
#define IMMED               0x80
#define INLINE              0x40
#define btwi(n,l,h)   ((l<=n) && (n<=h))
#define TOS           dstk[dsp]
#define NOS           dstk[dsp-1]
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]

enum { INTERPRET=0, COMPILE=1, BYE=999 };
typedef struct { ucell xt; byte fl; byte ln; char nm[NAME_SZ]; } DE_T;
typedef struct { char *name; ucell value; } NVP_T;

// These are defined by dwc-vm.c
extern void inner(ucell start);
extern void push(cell val);
extern cell pop();
extern void outer(const char *src);
extern void addLit(const char *name, cell val);
extern void dwcInit();
extern void dwcRun();
extern int nextWord();
extern DE_T *addToDict(char *w);
extern void iToA(cell n, cell b, cell w);
extern cell state;
extern DE_T *last;
extern char mem[];

// dwc-vm.c needs these to be defined
extern void zType(const char *str);
extern void emit(const char ch);
extern int  key();
extern int  keyboard_has_input();
extern char *strcpy(char *a, const char *b);
extern void *memcpy(void *dest, const void *src, cell n);
extern void *memmove(void *dest, const void *src, cell n);
extern int strlen(const char *a);
extern int strEqI(const char *a, const char *b);

#endif //  __DWC_VM_H__
