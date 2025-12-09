/*
    An Vu
    CSCI4500 Operating Systems
    November 24, 2025
*/

/*
    Program 3 Memory Management
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned int uint;

int verbose;        /* non-zero if -v option was specified */

int msize;          /* total memory size */

struct freg {       /* node for a free list region */
    struct freg *next;  /* ptr to next node */
    struct freg *prev;  /* ptr to previous node */
    uint size;      /* size of the region */
    uint addr;      /* starting address of the region */
} *freelist;        /* head of the free list */

/*---------------------------------------------------*/
/* Other data structures and globals.                */
/*---------------------------------------------------*/

struct alloc {              /* active allocation */
    struct alloc *next;
    struct alloc *prev;
    int rid;                /* request ID */
    uint size;              /* size allocated */
    uint addr;              /* starting address */
} *alloclist = NULL;

struct dreq {               /* deferred request node, FIFO queue */
    struct dreq *next;
    int rid;
    uint size;
} *dq_head = NULL, *dq_tail = NULL;

/* current request info */
int cur_rid = 0;
uint cur_rsize = 0;
int cur_rtype = 0;

/* address of most recent successful allocation */
uint cur_raddr = 0;

/* total allocated size */
uint total_alloc = 0;

/*---------------------------------------------*/
/* Display the free lists for all size blocks. */
/*---------------------------------------------*/
void show_free_list(void)
{
    uint size;
    int lx;
    struct freg *p;

    printf("  Free List\n");
    printf("-------------\n");
    if (freelist == NULL) {
        printf("(empty)\n");
        return;
    }
    printf("Address  Size\n");
    printf("-------  ----\n");
    for(p=freelist;p!=NULL;p=p->next) {
        printf("%7d  %4d\n", p->addr, p->size);
    }
    putchar('\n');
}

#define REQUEST_DEALLOCATE 0
#define REQUEST_ALLOCATE 1
/*--------------------------------------------------------------------*/
/* Get the next allocation/deallocation request and return 1, or      */
/* return 0 at end of file. On input error, diagnose and quit.        */
/* *rid == request ID                                                 */
/*  *rtype = 1 for allocation, 0 for deallocation                     */
/* *size = size of region requested (only if *rtype == 1). *su_20     */
/*--------------------------------------------------------------------*/
int get_request(int *rid, int *rtype, uint *size)
{
    int sfr;                /* scanf result */
    int request_id;
    char request_type;
    int request_size;

    sfr = scanf("%d",&request_id);
    if (sfr == EOF)
        return 0;
    if (sfr == 0) {
        fprintf(stderr,"trouble reading request ID from input.");
        exit(1);
    }
    sfr = scanf(" %c",&request_type);
    if (sfr != 1) {
        fprintf(stderr,"trouble reading request type from input.");
        exit(1);
    }

    if (request_type == '-') {      /* deallocation request */
        *rid = request_id;
        *rtype = REQUEST_DEALLOCATE;
        return 1;

    } else if (request_type == '+') {   /* allocation request */
        sfr = scanf("%d",&request_size);
        if (sfr != 1) {
            fprintf(stderr,"trouble reading allocation request size.");
            exit(1);
        }
        if (request_size < 1 || request_size > msize) {
            fprintf(stderr,"input allocation request size (%d) is invalid.",
                    request_size);
            exit(1);
        }
        *rid = request_id;
        *rtype = REQUEST_ALLOCATE;
        *size = (uint)request_size;
        return 1;
    }

    fprintf(stderr,"unrecognized request type: %c", request_type);
    return 0;       /* not normally reached */
}

/*---------------------------------------------*/
/* Append a deferred request to the FIFO queue */
/*---------------------------------------------*/
void add_deferred(int rid, uint size)
{
    struct dreq *d = (struct dreq *)malloc(sizeof(struct dreq));
    if (d == NULL) {
        fprintf(stderr, "Out of memory (deferred).\n");
        exit(1);
    }
    d->rid = rid;
    d->size = size;
    d->next = NULL;

    if (dq_head == NULL) {
        dq_head = dq_tail = d;
    } else {
        dq_tail->next = d;
        dq_tail = d;
    }
}

/*------------------------------------------*/
/* Try to perform allocation for a request. */
/* Return 1 if successful, 0 if deferred.   */
/*------------------------------------------*/
int allocate(void)
{
    struct freg *p, *best = NULL;

    /* find best-fit block: smallest block with size >= cur_rsize */
    for (p = freelist; p != NULL; p = p->next) {
        if (p->size >= cur_rsize) {
            if (best == NULL || p->size < best->size) {
                best = p;
            }
        }
    }

    if (best == NULL) {
        /* no block large enough, must defer */
        return 0;
    }

    /* allocate from the front of the best block */
    cur_raddr = best->addr;

    /* record allocation in alloclist */
    {
        struct alloc *a = (struct alloc *)malloc(sizeof(struct alloc));
        if (a == NULL) {
            fprintf(stderr, "Out of memory (alloclist).\n");
            exit(1);
        }
        a->rid = cur_rid;
        a->size = cur_rsize;
        a->addr = cur_raddr;

        /* insert at head of allocation list */
        a->prev = NULL;
        a->next = alloclist;
        if (alloclist != NULL)
            alloclist->prev = a;
        alloclist = a;
    }

    /* adjust free list */
    if (best->size == cur_rsize) {
        /* remove this node entirely */
        if (best->prev != NULL)
            best->prev->next = best->next;
        else
            freelist = best->next;

        if (best->next != NULL)
            best->next->prev = best->prev;

        free(best);
    } else {
        /* shrink region at the front */
        best->addr += cur_rsize;
        best->size -= cur_rsize;
    }

    total_alloc += cur_rsize;

    return 1;
}

/*--------------------------*/
/* Deallocate an allocation */
/*--------------------------*/
void deallocate(void)
{
    struct alloc *a;
    uint addr, size;

    /* find allocation by ID */
    for (a = alloclist; a != NULL && a->rid != cur_rid; a = a->next)
        ;

    if (a == NULL) {
        /* deallocation of unknown ID - nothing to free */
        if (verbose) {
            fprintf(stderr, "Warning: deallocate unknown ID %d\n", cur_rid);
        }
        return;
    }

    addr = a->addr;
    size = a->size;

    /* remove from allocation list */
    if (a->prev != NULL)
        a->prev->next = a->next;
    else
        alloclist = a->next;

    if (a->next != NULL)
        a->next->prev = a->prev;

    free(a);

    if (total_alloc >= size)
        total_alloc -= size;
    else
        total_alloc = 0;

    /* insert freed region into freelist in address order */
    {
        struct freg *p = freelist;
        struct freg *prev = NULL;
        struct freg *n;

        while (p != NULL && p->addr < addr) {
            prev = p;
            p = p->next;
        }

        n = (struct freg *)malloc(sizeof(struct freg));
        if (n == NULL) {
            fprintf(stderr, "Out of memory (free list).\n");
            exit(1);
        }
        n->addr = addr;
        n->size = size;
        n->prev = prev;
        n->next = p;

        if (prev != NULL)
            prev->next = n;
        else
            freelist = n;

        if (p != NULL)
            p->prev = n;

        /* coalesce with previous if adjacent */
        if (n->prev != NULL &&
            n->prev->addr + n->prev->size == n->addr) {

            struct freg *prevn = n->prev;
            prevn->size += n->size;
            prevn->next = n->next;
            if (n->next != NULL)
                n->next->prev = prevn;
            free(n);
            n = prevn;
        }

        /* coalesce with next if adjacent */
        if (n->next != NULL &&
            n->addr + n->size == n->next->addr) {

            struct freg *nextn = n->next;
            n->size += nextn->size;
            n->next = nextn->next;
            if (nextn->next != NULL)
                nextn->next->prev = n;
            free(nextn);
        }
    }
}

/*---------------------------------------------------*/
/* Try to allocate memory for the deferred requests. */
/*---------------------------------------------------*/
void dodef(void)
{
    struct dreq *prev = NULL;
    struct dreq *cur = dq_head;

    while (cur != NULL) {
        cur_rid = cur->rid;
        cur_rsize = cur->size;

        if (allocate()) {
            /* successful allocation of deferred request */
            printf("  Deferred request %d allocated; addr = 0x%08x. Total allocate size = %u\n",
                   cur_rid, cur_raddr, total_alloc);

            /* remove this deferred node */
            struct dreq *tmp = cur;
            if (prev != NULL)
                prev->next = cur->next;
            else
                dq_head = cur->next;

            cur = cur->next;

            if (tmp == dq_tail)
                dq_tail = prev;

            free(tmp);
        } else {
            /* still cannot satisfy this one */
            prev = cur;
            cur = cur->next;
        }
    }
}

int main(int argc, char *argv[])
{
    int rid;            /* request ID */
    int rtype;          /* request type */
    uint rsize;         /* request size */
    int ok;             /* non-zero if allocation succeeded */

    /*------------*/
    /* Read msize */
    /*------------*/
    scanf("%d",&msize);

    /*-----------------------------------------------------------*/
    /* Construct an freelist with one entry, size msize, addr 0. */
    /*-----------------------------------------------------------*/
    freelist = (struct freg *)malloc(sizeof(struct freg));
    if (freelist == NULL) {
        fprintf(stderr,"Out of memory.\n");
        exit(1);
    }

    freelist->addr = 0;
    freelist->size = msize;
    freelist->prev = NULL;
    freelist->next = NULL;

    /*----------------------------------------------------------*/
    /* Initialize other data structures.                        */
    /*----------------------------------------------------------*/
    alloclist = NULL;
    dq_head = dq_tail = NULL;
    cur_rid = 0;
    cur_rsize = 0;
    cur_rtype = 0;
    cur_raddr = 0;
    total_alloc = 0;

    /*-----------------------*/
    /* Process the requests. */
    /*-----------------------*/
    while (get_request(&rid, &rtype, &rsize)) {
        printf("Request ID %d: ", rid);
        if (rtype == REQUEST_ALLOCATE)
            printf("allocate %d unit%s.\n", rsize, rsize == 1 ? "" : "s");
        else
            printf("deallocate.\n");

        /* make current request visible to helpers */
        cur_rid = rid;
        cur_rtype = rtype;
        cur_rsize = rsize;

        /*----------------------*/
        /* Process the request. */
        /*----------------------*/
        if (rtype == REQUEST_ALLOCATE) {    /* allocation request */
            ok = allocate();
            if (ok) {           /* request was successful */
                printf("  Success; addr = 0x%08x. Total allocate size = %u\n",
                       cur_raddr, total_alloc);
            } else {            /* defer the request */
                printf("  Request deferred. Total allocate size = %u\n",
                       total_alloc);
                add_deferred(rid, rsize);
            }
        } else {            /* deallocation request */
            deallocate();       /* do the deallocation */
            printf("  Success. Total allocated size = %u\n", total_alloc);

            /*----------------------------------------------*/
            /* Try to perform deferred allocation requests. */
            /*----------------------------------------------*/
            dodef();
        }
    }

    return 0;
}
