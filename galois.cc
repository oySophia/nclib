#include <pthread.h>
#include <string>
#include <cstring>
#include <vector>
#include <list>
#include <algorithm>
#include <map>
#include <list>
#include <set>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "galois.h"
using namespace std;
/* Mutex needs to be initialized in all programs that use it (?) */

#define NONE (10)
#define TABLE (11)
#define SHIFT (12)
#define LOGS (13)
#define SPLITW8 (14)
#define rgw32_mask(v) ((v) & 0x80000000)

static int prim32 = -1;
static uint64_t prim32_64 = -1UL;
static uint64_t mask32_1_64 = -1UL;
static uint64_t mask32_2_64 = -1UL;
static int prim04 = -1;
static int mask04_1 = -1;
static int mask04_2 = -1;
static uint64_t prim04_64 = -1UL;
static uint64_t mask04_1_64 = -1UL;
static uint64_t mask04_2_64 = -1UL;
static int prim08 = -1;
static int mask08_1 = -1;
static int mask08_2 = -1;
static uint64_t prim08_64 = -1UL;
static uint64_t mask08_1_64 = -1UL;
static uint64_t mask08_2_64 = -1UL;
static int prim16 = -1;
static int mask16_1 = -1;
static int mask16_2 = -1;
static uint64_t prim16_64 = -1UL;
static uint64_t mask16_1_64 = -1UL;
static uint64_t mask16_2_64 = -1UL;
static uint64_t prim64 = 0x1b;
//static uint64_t prim64_2 = 0x232ecee416c4f011;
static int prim_poly[33] = 
{ 0, 
  /*  1 */     1, 
  /*  2 */    07,
  /*  3 */    013,
  /*  4 */    023,
  /*  5 */    045,
  /*  6 */    0103,
  /*  7 */    0211,
  /*  8 */    0435,
  /*  9 */    01021,
  /* 10 */    02011,
  /* 11 */    04005,
  /* 12 */    010123,
  /* 13 */    020033,
  /* 14 */    042103,
  /* 15 */    0100003,
  /* 16 */    0210013,
  /* 17 */    0400011,
  /* 18 */    01000201,
  /* 19 */    02000047,
  /* 20 */    04000011,
  /* 21 */    010000005,
  /* 22 */    020000003,
  /* 23 */    040000041,
  /* 24 */    0100000207,
  /* 25 */    0200000011,
  /* 26 */    0400000107,
  /* 27 */    01000000047,
  /* 28 */    02000000011,
  /* 29 */    04000000005,
  /* 30 */    010040000007,
  /* 31 */    020000000011, 
  /* 32 */    00020000007 };  /* Really 40020000007, but we're omitting the high order bit */

static int mult_type[33] = 
{ NONE, 
  /*  1 */   TABLE, 
  /*  2 */   TABLE,
  /*  3 */   TABLE,
  /*  4 */   TABLE,
  /*  5 */   TABLE,
  /*  6 */   TABLE,
  /*  7 */   TABLE,
  /*  8 */   TABLE,
  /*  9 */   TABLE,
  /* 10 */   LOGS,
  /* 11 */   LOGS,
  /* 12 */   LOGS,
  /* 13 */   LOGS,
  /* 14 */   LOGS,
  /* 15 */   LOGS,
  /* 16 */   LOGS,
  /* 17 */   LOGS,
  /* 18 */   LOGS,
  /* 19 */   LOGS,
  /* 20 */   LOGS,
  /* 21 */   LOGS,
  /* 22 */   LOGS,
  /* 23 */   SHIFT,
  /* 24 */   SHIFT,
  /* 25 */   SHIFT,
  /* 26 */   SHIFT,
  /* 27 */   SHIFT,
  /* 28 */   SHIFT,
  /* 29 */   SHIFT,
  /* 30 */   SHIFT,
  /* 31 */   SHIFT,
  /* 32 */   SPLITW8 };

static int nw[33] = { 0, (1 << 1), (1 << 2), (1 << 3), (1 << 4), 
  (1 << 5), (1 << 6), (1 << 7), (1 << 8), (1 << 9), (1 << 10),
  (1 << 11), (1 << 12), (1 << 13), (1 << 14), (1 << 15), (1 << 16),
  (1 << 17), (1 << 18), (1 << 19), (1 << 20), (1 << 21), (1 << 22),
  (1 << 23), (1 << 24), (1 << 25), (1 << 26), (1 << 27), (1 << 28),
  (1 << 29), (1 << 30), (1 << 31), -1 };

static int nwm1[33] = { 0, (1 << 1)-1, (1 << 2)-1, (1 << 3)-1, (1 << 4)-1, 
  (1 << 5)-1, (1 << 6)-1, (1 << 7)-1, (1 << 8)-1, (1 << 9)-1, (1 << 10)-1,
  (1 << 11)-1, (1 << 12)-1, (1 << 13)-1, (1 << 14)-1, (1 << 15)-1, (1 << 16)-1,
  (1 << 17)-1, (1 << 18)-1, (1 << 19)-1, (1 << 20)-1, (1 << 21)-1, (1 << 22)-1,
  (1 << 23)-1, (1 << 24)-1, (1 << 25)-1, (1 << 26)-1, (1 << 27)-1, (1 << 28)-1,
  (1 << 29)-1, (1 << 30)-1, 0x7fffffff, 0xffffffff };

static int *galois_log_tables[33] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static int *galois_ilog_tables[33] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static int *galois_mult_tables[33] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static int *galois_div_tables[33] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

//static uint64_t *galois_log_table_64 = NULL;
//static uint64_t *galois_ilog_table_64 = NULL;

/* Special case for w = 32 */
static int *galois_split_w8[7] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
/* Special case for w = 64 */
//static uint64_t *galois_split_w16_log_table[4] = { NULL, NULL, NULL, NULL }; 
//static uint64_t *galois_split_w16_ilog_table[4] = { NULL, NULL, NULL, NULL };

static pthread_mutex_t galois_lock;

int galois_create_log_tables_64(){
    /* dead code 
    uint64_t x;
    int i, j;
    try{
        galois_log_table_64 = new uint64_t [65536];
        galois_ilog_table_64 = new uint64_t [65536];
    }catch(bad_alloc &e){
        if(galois_log_table_64 != NULL) delete galois_log_table_64;
        if(galois_ilog_table_64 != NULL) delete galois_ilog_table_64;
        return -1;
    }
    */
    return 0;
}

int galois_create_log_tables(int w)
{
  int j, b;

  if (w > 30) return -1;
  if (galois_log_tables[w] != NULL) return 0;

	try {
		galois_log_tables[w] = new int [nw[w]];
		galois_ilog_tables[w] = new int [nw[w] * 3];
	} catch (bad_alloc &e) {
		if (galois_log_tables[w] != NULL) delete galois_log_tables[w];
		if (galois_ilog_tables[w] != NULL) delete galois_ilog_tables[w];
		galois_log_tables[w] = NULL;
		galois_ilog_tables[w] = NULL;
		return -1;
	}

  for (j = 0; j < nw[w]; j++) {
    galois_log_tables[w][j] = nwm1[w];
    galois_ilog_tables[w][j] = 0;
  }

	/* Fill in log and ilog tables, doing single left shifts and xoring
		 with the prim poly if necessary */
  b = 1;
  for (j = 0; j < nwm1[w]; j++) {
    if (galois_log_tables[w][b] != nwm1[w]) {
      fprintf(stderr, "Galois_create_log_tables Error: j=%d, b=%d, B->J[b]=%d, J->B[j]=%d (0%o)\n",
          j, b, galois_log_tables[w][b], galois_ilog_tables[w][j], (b << 1) ^ prim_poly[w]);
      exit(1);
    }
    galois_log_tables[w][b] = j;
    galois_ilog_tables[w][j] = b;
    b = b << 1;
    if (b & nw[w]) b = (b ^ prim_poly[w]) & nwm1[w];
  }

	/* Triple the size of ilog tables, allowing all multiplication
		 and division to be done without testing ilog range */
  for (j = 0; j < nwm1[w]; j++) {
    galois_ilog_tables[w][j+nwm1[w]] = galois_ilog_tables[w][j];
    galois_ilog_tables[w][j+nwm1[w]*2] = galois_ilog_tables[w][j];
  }
  galois_ilog_tables[w]+=nwm1[w];
  return 0;
}

int galois_logtable_multiply(int x, int y, int w)
{
  int sum_j;

  if (x == 0 || y == 0) return 0;

  sum_j = galois_log_tables[w][x] + galois_log_tables[w][y];
  /* if (sum_j >= nwm1[w]) sum_j -= nwm1[w];    Don't need to do this, 
     because we replicate the ilog table twice.  */
  return galois_ilog_tables[w][sum_j];
}

int galois_logtable_divide(int x, int y, int w)
{
  int sum_j;
  int z;

  if (y == 0) return -1;
  if (x == 0) return 0;
  sum_j = galois_log_tables[w][x] - galois_log_tables[w][y];
  /* if (sum_j < 0) sum_j += nwm1[w];   Don't need to do this, 
     because we replicate the ilog table twice.   */
  z = galois_ilog_tables[w][sum_j];
  return z;
}

int galois_create_mult_tables(int w)
{
  int j, x, y, logx;
  //int *mtable, *dtable;

  if (w >= 14) return -1;

  if (galois_mult_tables[w] != NULL) return 0;

	try {
		galois_mult_tables[w] = new int [nw[w] * nw[w]];
		galois_div_tables[w] = new int [nw[w] * nw[w]];
	} catch (bad_alloc &e) {
		if (galois_mult_tables[w] != NULL) delete galois_mult_tables[w];
		if (galois_div_tables[w] != NULL) delete galois_div_tables[w];
		galois_mult_tables[w] = NULL;
		galois_div_tables[w] = NULL;
    return -1;
	}
	/*
  galois_mult_tables[w] = (int *) malloc(sizeof(int) * nw[w] * nw[w]);
  if (galois_mult_tables[w] == NULL) return -1;

  galois_div_tables[w] = (int *) malloc(sizeof(int) * nw[w] * nw[w]);
  if (galois_div_tables[w] == NULL) {
    free(galois_mult_tables[w]);
    galois_mult_tables[w] = NULL;
    return -1;
  } */
  /* Don't need to lock because the mutex should be help at any point during a
     create function call.  This brings up the issue of users calling create
     functions directly */
  if (galois_log_tables[w] == NULL) {
    if (galois_create_log_tables(w) < 0) {
      free(galois_mult_tables[w]);
      free(galois_div_tables[w]);
      galois_mult_tables[w] = NULL;
      galois_div_tables[w] = NULL;
      return -1;
    }
  }

  /* Set mult/div tables for x = 0 */
  j = 0;
  galois_mult_tables[w][j] = 0;   /* y = 0 */
  galois_div_tables[w][j] = -1;
  j++;
  for (y = 1; y < nw[w]; y++) {   /* y > 0 */
    galois_mult_tables[w][j] = 0;
    galois_div_tables[w][j] = 0;
    j++;
  }

  for (x = 1; x < nw[w]; x++) {  /* x > 0 */
    galois_mult_tables[w][j] = 0; /* y = 0 */
    galois_div_tables[w][j] = -1;
    j++;
    logx = galois_log_tables[w][x];
    for (y = 1; y < nw[w]; y++) {  /* y > 0 */
      galois_mult_tables[w][j] = galois_ilog_tables[w][logx+galois_log_tables[w][y]];
      galois_div_tables[w][j] = galois_ilog_tables[w][logx-galois_log_tables[w][y]];
      j++;
    }
  }
  return 0;
}

int galois_log(int value, int w)
{
  pthread_mutex_lock(&galois_lock);
  if (galois_log_tables[w] == NULL) {
    if (galois_create_log_tables(w) < 0) {
      fprintf(stderr, "Error: galois_log - w is too big.  Sorry\n");
      exit(1);
    }
  }
  pthread_mutex_unlock(&galois_lock);
  return galois_log_tables[w][value];
}

int galois_ilog(int value, int w)
{  
  pthread_mutex_lock(&galois_lock);
  if (galois_ilog_tables[w] == NULL) {
    if (galois_create_log_tables(w) < 0) {
      fprintf(stderr, "Error: galois_ilog - w is too big.  Sorry\n");
      exit(1);
    }
  }
  pthread_mutex_unlock(&galois_lock);
  return galois_ilog_tables[w][value];
}

int galois_shift_multiply(int x, int y, int w)
{
  int prod;
  int i, ind;
  //int k;
  int scratch[33];

	/* set values of scratch to be log values, starting at y */
  prod = 0;
  for (i = 0; i < w; i++) {
    scratch[i] = y;
    if (y & (1 << (w-1))) {
      y = y << 1;
      y = (y ^ prim_poly[w]) & nwm1[w];
    } else {
      y = y << 1;
    }
  }
	/* do magic */
  for (i = 0; i < w; i++) {
    ind = (1 << i);
    if (ind & x) {
       prod = prod^scratch[i]; 
        /*
      j = 1;
      for (k = 0; k < w; k++) {
        prod = prod ^ (j & scratch[i]);
        j = (j << 1);
      }
      */
    }
  }
  return prod;
}

uint64_t galois_shift_multiply_64(uint64_t x, uint64_t y){
    uint64_t prod = 0;
    uint64_t yy[64];
    int i;
    uint64_t ind;
    
    yy[0] = y;
    for(i = 1; i < 64; i++){
        yy[i] = yy[i-1];
        if(yy[i]&0x8000000000000000){
            yy[i] = yy[i]<<1;
            yy[i] = yy[i]^prim64;
        }else{
            yy[i] = yy[i]<<1;
        }
    }
    
    for(i = 0; i < 64; i++){
        ind = 1UL << i;
        if(ind & x){
            prod = prod^yy[i];
        }
    }

    return prod;
}

int galois_single_multiply(int x, int y, int w)
{
  int sum_j;
  int z;

  if (x == 0 || y == 0) return 0;

  if (mult_type[w] == TABLE) {
    pthread_mutex_lock(&galois_lock);
    if (galois_mult_tables[w] == NULL) {
      if (galois_create_mult_tables(w) < 0) {
        fprintf(stderr, "ERROR -- cannot make multiplication tables for w=%d\n", w);
        exit(1);
      }
    }
    pthread_mutex_unlock(&galois_lock);
    return galois_mult_tables[w][(x<<w)|y];
  } else if (mult_type[w] == LOGS) {
    pthread_mutex_lock(&galois_lock);
    if (galois_log_tables[w] == NULL) {
      if (galois_create_log_tables(w) < 0) {
        fprintf(stderr, "ERROR -- cannot make log tables for w=%d\n", w);
        exit(1);
      }
    }
    pthread_mutex_unlock(&galois_lock);
    sum_j = galois_log_tables[w][x] + galois_log_tables[w][y];
    z = galois_ilog_tables[w][sum_j];
    return z;
  } else if (mult_type[w] == SPLITW8) {
    pthread_mutex_lock(&galois_lock);
    if (galois_split_w8[0] == NULL) {
      if (galois_create_split_w8_tables() < 0) {
        fprintf(stderr, "ERROR -- cannot make log split_w8_tables for w=%d\n", w);
        exit(1);
      }
    }
    pthread_mutex_unlock(&galois_lock);
    return galois_split_w8_multiply(x, y);
  } else if (mult_type[w] == SHIFT) {
    return galois_shift_multiply(x, y, w);
  }
  fprintf(stderr, "Galois_single_multiply - no implementation for w=%d\n", w);
  exit(1);
}

int galois_multtable_multiply(int x, int y, int w)
{
  return galois_mult_tables[w][(x<<w)|y];
}

int galois_single_divide(int a, int b, int w)
{
  int sum_j;

  if (mult_type[w] == TABLE) {
    pthread_mutex_lock(&galois_lock);
    if (galois_div_tables[w] == NULL) {
      if (galois_create_mult_tables(w) < 0) {
        fprintf(stderr, "ERROR -- cannot make multiplication tables for w=%d\n", w);
        exit(1);
      }
    }
    pthread_mutex_unlock(&galois_lock);
    return galois_div_tables[w][(a<<w)|b];
  } else if (mult_type[w] == LOGS) {
    if (b == 0) return -1;
    if (a == 0) return 0;
    pthread_mutex_lock(&galois_lock);
    if (galois_log_tables[w] == NULL) {
      if (galois_create_log_tables(w) < 0) {
        fprintf(stderr, "ERROR -- cannot make log tables for w=%d\n", w);
        exit(1);
      }
    }
    pthread_mutex_unlock(&galois_lock);
    sum_j = galois_log_tables[w][a] - galois_log_tables[w][b];
    return galois_ilog_tables[w][sum_j];
  } else {
    if (b == 0) return -1;
    if (a == 0) return 0;
    sum_j = galois_inverse(b, w);
    return galois_single_multiply(a, sum_j, w);
  }
  fprintf(stderr, "Galois_single_divide - no implementation for w=%d\n", w);
  exit(1);
}

uint64_t galois_single_divide_64(uint64_t a, uint64_t b){
    uint64_t tmp;

    if(b == 0) return -1;
    if(a == 0) return 0;
    tmp = galois_shift_inverse_64(b);
    
    return galois_shift_multiply_64(a, tmp);
}

int galois_shift_divide(int a, int b, int w)
{
  int inverse;

  if (b == 0) return -1;
  if (a == 0) return 0;
  inverse = galois_shift_inverse(b, w);
  return galois_shift_multiply(a, inverse, w);
}

int galois_multtable_divide(int x, int y, int w)
{
  return galois_div_tables[w][(x<<w)|y];
}

void galois_w08_region_multiply(unsigned char *region,       /* Region to multiply */
                                         int multby,         /* Number to multiply by */
                                         int nbytes,         /* Number of bytes in region */
                                unsigned char *r2,           /* If r2 != NULL, products go here.  
                                                                Otherwise region is overwritten */
                                        int add)            /* If (r2 != NULL && add) the produce is XOR'd with r2 */
{
  unsigned char *ur1, *ur2, *cp;
  //uint64_t *ut1, *ut2, ut;
  unsigned char prod;
  int i, srow, j;
  uint64_t l, *lp2;
  unsigned char *lp;
  int sol;

  ur1 = (unsigned char *) region;
  ur2 = (r2 == NULL) ? ur1 : (unsigned char *) r2;

  /* This is used to test its performance with respect to just calling galois_single_multiply 
     if (r2 == NULL || !add) {
     for (i = 0; i < nbytes; i++) ur2[i] = galois_single_multiply(ur1[i], multby, 8);
     } else {
     for (i = 0; i < nbytes; i++) {
     ur2[i] = (ur2[i]^galois_single_multiply(ur1[i], multby, 8));
     }
     }
   */

  pthread_mutex_lock(&galois_lock);
  if (galois_mult_tables[8] == NULL) {
    if (galois_create_mult_tables(8) < 0) {
      fprintf(stderr, "galois_08_region_multiply -- couldn't make multiplication tables\n");
      exit(1);
    }
  }          
  pthread_mutex_unlock(&galois_lock);

	/* With galois w==08, we multiply single characters (int value 0 - 255).
		 fixed width uint64_t are used as the packet size, but it is guaranteed
		 to be that size at this point */
  srow = multby * nw[8];
  if (r2 == NULL || !add) {
    for (i = 0; i < nbytes; i++) {
      prod = galois_mult_tables[8][srow+ur1[i]];
      ur2[i] = prod;
    }
  } else {
    /*************** part 1 *********************/
    sol = sizeof(uint64_t);
    lp2 = &l;
    lp = (unsigned char *) lp2;
    for (i = 0; i < nbytes; i += sol) {
      cp = ur2+i;
      lp2 = (uint64_t *) cp;
      for (j = 0; j < sol; j++) {
        prod = galois_mult_tables[8][srow+ur1[i+j]];
        lp[j] = prod;
      }
      *lp2 = (*lp2) ^ l;
    }
    /********************************************/
    /*************** part 2 *********************
        for (i = 0; i < nbytes; i++) {
            prod = galois_mult_tables[8][srow+ur1[i]];
            ur2[i] = ur2[i]^prod;
        }
    ********************************************/
  }
  return;
}

void galois_w16_region_multiply(unsigned char *region,      /* Region to multiply */
                                          int multby,       /* Number to multiply by */
                                          int nbytes,       /* Number of bytes in region */
                                unsigned char *r2,          /* If r2 != NULL, products go here */
                                          int add)
{
  uint16_t *ur1, *ur2; //uint16_t *ur1, *ur2, *cp;
  int prod;
  int i, log1,  log2; //add j for original codes
  uint64_t l = 0, *lp2, *lptop;
  uint16_t *lp;
  uint16_t te = 0;
  int sol;

  ur1 = (uint16_t *) region;
  ur2 = (r2 == NULL) ? ur1 : (uint16_t *) r2;
  nbytes /= sizeof(uint16_t);

  /* This is used to test its performance with respect to just calling galois_single_multiply */
  /*
     if (r2 == NULL || !add) {
     for (i = 0; i < nbytes; i++) ur2[i] = galois_single_multiply(ur1[i], multby, 16);
     } else {
     for (i = 0; i < nbytes; i++) {
     ur2[i] = (ur2[i]^galois_single_multiply(ur1[i], multby, 16));
     }
     }
     return;
   */

  if (multby == 0) {
    if (!add) {
      lp2 = (uint64_t *) ur2;
      ur2 += nbytes;
      lptop = (uint64_t *) ur2;
      while (lp2 < lptop) { *lp2 = 0; lp2++; }
    }
    return;
  }

  pthread_mutex_lock(&galois_lock);
  if (galois_log_tables[16] == NULL) {
    if (galois_create_log_tables(16) < 0) {
      fprintf(stderr, "galois_16_region_multiply -- couldn't make log tables\n");
      exit(1);
    }
  }
  pthread_mutex_unlock(&galois_lock);
  log1 = galois_log_tables[16][multby];

	/* For w==16, we use fixed width uint16_t with int values 0 - 2^16-1
		 */
  if (r2 == NULL || !add) {
    for (i = 0; i < nbytes; i++) {
      if (ur1[i] == 0) {
        ur2[i] = 0;
      } else {
        prod = galois_log_tables[16][ur1[i]] + log1;
        ur2[i] = galois_ilog_tables[16][prod];
      }
    }
  } else {

    sol = sizeof(uint64_t)/2;
    lp2 = &l;
    lp = (uint16_t *) &te;
      /*
    for (i = 0; i < nbytes; i += sol) {
      cp = ur2+i;
      lp2 = (uint64_t *) cp;
      for (j = 0; j < sol; j++) {
        if (ur1[i+j] == 0) {
          *(lp+j) = 0;
        } else {
          log2 = galois_log_tables[16][ur1[i+j]];
          prod = log2 + log1;
          *(lp+j) = galois_ilog_tables[16][prod];
        }
      }
      *lp2 = (*lp2) ^ l;
    }*/
    for(i = 0; i < nbytes; ++i){
        if(ur1[i] == 0){
            ur2[i] = 0;
        }else{
          log2 = galois_log_tables[16][ur1[i]];
          prod = log2 + log1;
          ur2[i] = ur2[i] ^ galois_ilog_tables[16][prod];
        }
    }

  }
  return;
}

void galois_invert_binary_matrix_64(uint64_t *mat, uint64_t *inv){
  int i, j;
  uint64_t tmp;
  int rows = 64;
  int cols = 64;

  for (i = 0; i < cols; i++){
      inv[i] = (1UL << i);
  }

  for (i = 0; i < cols; i++) {
    if ((mat[i] & (1UL << i)) == 0) {    
      for (j = i+1; j < rows && (mat[j] & (1UL << i)) == 0; j++) ;
      if(j == rows){
        printf("i = %u, j = %u\n", i, j);
        fprintf(stderr, "galois_invert_matrix_64: Matrix not invertible!!\n");
        exit(1);
      }
      tmp = mat[i]; mat[i] = mat[j]; mat[j] = tmp;
      tmp = inv[i]; inv[i] = inv[j]; inv[j] = tmp;
    }

    for (j = i+1; j != rows; j++) {
      if ((mat[j] & (1UL << i)) != 0) {
        mat[j] ^= mat[i];
        inv[j] ^= inv[i];
      }
    }
  }

  for (i = rows-1; i >= 0; i--) {
    for (j = 0; j < i; j++) {
      if (mat[j] & (1UL << i)) {
        mat[j] ^= mat[i];
        inv[j] ^= inv[i];
      }
    }
  }
/*  for(i=0;i<rows;i++){
    for(j=0;j<cols;j++){
        printf("%d",((mat[i]&(1UL<<j))?1:0));
    }
    printf("\n");
  }*/
}

void galois_invert_binary_matrix(int *mat, int *inv, int rows)
{
  int cols, i, j;
  int tmp;

  cols = rows;

  for (i = 0; i < rows; i++) inv[i] = (1 << i);

  /* First -- convert into upper triangular */

  for (i = 0; i < cols; i++) {

    /* Swap rows if we ave a zero i,i element.  If we can't swap, then the 
       matrix was not invertible */

    if ((mat[i] & (1 << i)) == 0) {
      for (j = i+1; j < rows && (mat[j] & (1 << i)) == 0; j++) ;
      if (j == rows) {
        fprintf(stderr, "galois_invert_matrix: Matrix not invertible!!\n");
        exit(1);
      }
      tmp = mat[i]; mat[i] = mat[j]; mat[j] = tmp;
      tmp = inv[i]; inv[i] = inv[j]; inv[j] = tmp;
    }

    /* Now for each j>i, add A_ji*Ai to Aj */
    for (j = i+1; j != rows; j++) {
      if ((mat[j] & (1 << i)) != 0) {
        mat[j] ^= mat[i];
        inv[j] ^= inv[i];
      }
    }
  }

/*  for(i=0;i<rows;i++){
    for(j=0;j<cols;j++){
        printf("%d",((mat[i]&(1<<j))?1:0));
    }
    printf("\n");
  }*/
  /* Now the matrix is upper triangular.  Start at the top and multiply down */

  for (i = rows-1; i >= 0; i--) {
    for (j = 0; j < i; j++) {
      if (mat[j] & (1 << i)) {
       /* mat[j] ^= mat[i]; */
        inv[j] ^= inv[i];
      }
    }
  }
/*  for(i=0;i<rows;i++){
    for(j=0;j<cols;j++){
        printf("%d",((mat[i]&(1<<j))?1:0));
    }
    printf("\n");
  }*/
}

int galois_inverse(int y, int w)
{

  if (y == 0) return -1;
  if (mult_type[w] == SHIFT || mult_type[w] == SPLITW8) return galois_shift_inverse(y, w);
  return galois_single_divide(1, y, w);
}

int galois_shift_inverse(int y, int w)
{
  int mat2[32];
  int inv2[32];
  int i;       //int ind, i, j, k, prod;

  for (i = 0; i < w; i++) {
    mat2[i] = y;

    if (y & nw[w-1]) {
      y = y << 1;
      y = (y ^ prim_poly[w]) & nwm1[w];
    } else {
      y = y << 1;
    }
  }

  galois_invert_binary_matrix(mat2, inv2, w);

  return inv2[0];
}

uint64_t galois_shift_inverse_64(uint64_t x){
    uint64_t mat[64];
    uint64_t inv[64];
    int i;

    for(i = 0; i < 64; i++){
        mat[i] = x;
        if(x&0x8000000000000000){
            x = x << 1;
            x = x ^ prim64;
        }else{
            x = x << 1;
        }
    }
    galois_invert_binary_matrix_64(mat, inv);

    return inv[0];
}

int *galois_get_mult_table(int w)
{
  pthread_mutex_lock(&galois_lock);
  if (galois_mult_tables[w] == NULL) {
    if (galois_create_mult_tables(w)) {
      return NULL;
    }
  }
  pthread_mutex_unlock(&galois_lock);
  return galois_mult_tables[w];
}

int *galois_get_div_table(int w)
{
  pthread_mutex_lock(&galois_lock);
  if (galois_mult_tables[w] == NULL) {
    if (galois_create_mult_tables(w)) {
      return NULL;
    }
  }
  pthread_mutex_unlock(&galois_lock);
  return galois_div_tables[w];
}

int *galois_get_log_table(int w)
{
  pthread_mutex_lock(&galois_lock);
  if (galois_log_tables[w] == NULL) {
    if (galois_create_log_tables(w)) {
      return NULL;
    }
  }
  pthread_mutex_unlock(&galois_lock);
  return galois_log_tables[w];
}

int *galois_get_ilog_table(int w)
{
  pthread_mutex_lock(&galois_lock);
  if (galois_ilog_tables[w] == NULL) {
    if (galois_create_log_tables(w)) {
      return NULL;
    }
  }
  pthread_mutex_unlock(&galois_lock);
  return galois_ilog_tables[w];
}

void galois_w32_region_multiply(unsigned char *region,      /* Region to multiply */
                                          int multby,       /* Number to multiply by */
                                          int nbytes,        /* Number of bytes in region */
                                 unsigned char *r2,          /* If r2 != NULL, products go here */
                                          int add)
{
  uint32_t *ur1, *ur2, *ur2top;
  //uint64_t *lp2, *lptop;
  int i, j, a, b, accumulator, i8, j8, k;
  int acache[4];

  ur1 = (uint32_t *) region;
  ur2 = (r2 == NULL) ? ur1 : (uint32_t *) r2;
  nbytes /= sizeof(uint32_t);
  ur2top = ur2 + nbytes;

  pthread_mutex_lock(&galois_lock);
  if (galois_split_w8[0]== NULL) {
    if (galois_create_split_w8_tables() < 0) {
      fprintf(stderr, "galois_32_region_multiply -- couldn't make split multiplication tables\n");
      exit(1);
    }
  }
  pthread_mutex_unlock(&galois_lock);

  /* If we're overwriting r2, then we can't do better than just calling split_multiply.
     We'll inline it here to save on the procedure call overhead */

  i8 = 0;
  for (i = 0; i < 4; i++) {
    acache[i] = (((multby >> i8) & 255) << 8);
    i8 += 8;
  }

	/* This works */
  if (!add) {
    for (k = 0; k < nbytes; k++) {
      accumulator = 0;
      for (i = 0; i < 4; i++) {
        a = acache[i];
        j8 = 0;
        for (j = 0; j < 4; j++) {
          b = ((ur1[k] >> j8) & 255);
          accumulator ^= galois_split_w8[i+j][a|b];
          j8 += 8;
        }
      }
      ur2[k] = accumulator;
    }
  } else {
    for (k = 0; k < nbytes; k++) {
      accumulator = 0;
      for (i = 0; i < 4; i++) {
        a = acache[i];
        j8 = 0;
        for (j = 0; j < 4; j++) {
          b = ((ur1[k] >> j8) & 255);
          accumulator ^= galois_split_w8[i+j][a|b];
          j8 += 8;
        }
      }
      ur2[k] = (ur2[k] ^ accumulator);
    }
  }
  return;
}

void galois_region_xor(   unsigned char *r1,         /* Region 1 */
                          unsigned char *r2,         /* Region 2 */
                          unsigned char *r3,         /* Sum region (r3 = r1 ^ r2) -- can be r1 or r2 */
                          int nbytes)       /* Number of bytes in region */
{
  /* nbytes must be a multiply of uint64_t (8 bytes) */
  uint64_t *l1;
  uint64_t *l2;
  uint64_t *l3;
  uint64_t *ltop;
  unsigned char *ctop;

  ctop = r1 + nbytes;
  ltop = (uint64_t *) ctop;
  l1 = (uint64_t *) r1;
  l2 = (uint64_t *) r2;
  l3 = (uint64_t *) r3;

  while (l1 < ltop) {
    *l3 = ((*l1)  ^ (*l2));
    l1++;
    l2++;
    l3++;
  }
}
    
void galois_region_xor_1k(unsigned char *r1,         /* Region 1 */
                          unsigned char *r2,         /* Region 2 */
                          unsigned char *r3,         /* Sum region (r3 = r1 ^ r2) -- can be r1 or r2 */
                          unsigned long  nbytes)       /* Number of bytes in region */
{
  /* nbytes must be a multiply of 1KB (1024 bytes) */
  uint64_t *l1;
  uint64_t *l2;
  uint64_t *l3;
  uint64_t *ltop;
  unsigned char *ctop;
  int i;
  int loop = nbytes/128;

  ctop = r1 + nbytes;
  ltop = (uint64_t *) ctop;
  l1 = (uint64_t *) r1;
  l2 = (uint64_t *) r2;
  l3 = (uint64_t *) r3;

  for(i = 0; i < loop; i = i+1) {
    *(l3) = (*(l1))^(*(l2));
    *(l3+1) = (*(l1+1))^(*(l2+1));
    *(l3+2) = (*(l1+2))^(*(l2+2));
    *(l3+3) = (*(l1+3))^(*(l2+3));
    *(l3+4) = (*(l1+4))^(*(l2+4));
    *(l3+5) = (*(l1+5))^(*(l2+5));
    *(l3+6) = (*(l1+6))^(*(l2+6));
    *(l3+7) = (*(l1+7))^(*(l2+7));
    *(l3+8) = (*(l1+8))^(*(l2+8));
    *(l3+9) = (*(l1+9))^(*(l2+9));
    *(l3+10) = (*(l1+10))^(*(l2+10));
    *(l3+11) = (*(l1+11))^(*(l2+11));
    *(l3+12) = (*(l1+12))^(*(l2+12));
    *(l3+13) = (*(l1+13))^(*(l2+13));
    *(l3+14) = (*(l1+14))^(*(l2+14));
    *(l3+15) = (*(l1+15))^(*(l2+15));
    l3 = l3 + 16;
    l2 = l2 + 16;
    l1 = l1 + 16;
  }
}


int galois_create_split_w8_tables()
{
  int p1, p2, i, j, p1elt, p2elt, index, ishift, jshift, *table;

  if (galois_split_w8[0] != NULL) return 0;

  if (galois_create_mult_tables(8) < 0) return -1;

	try {
		for (i = 0; i < 7; i++) {
			galois_split_w8[i] = new int [( 1 << 16 )];
			/*galois_split_w8[i] = (int *) malloc(sizeof(int) * (1 << 16));
			if (galois_split_w8[i] == NULL) {
				for (i--; i >= 0; i--) free(galois_split_w8[i]);
				return -1;
			} */
		}
	} catch (bad_alloc &e) {
		for (i = 0; i < 7; i++) {
			if (galois_split_w8[i] != NULL) delete galois_split_w8[i];
			galois_split_w8[i] = NULL;
		}
		return -1;
	}

  for (i = 0; i < 4; i += 3) {
    ishift = i * 8;
    for (j = ((i == 0) ? 0 : 1) ; j < 4; j++) {
      jshift = j * 8;
      table = galois_split_w8[i+j];
      index = 0;
      for (p1 = 0; p1 < 256; p1++) {
        p1elt = (p1 << ishift);
        for (p2 = 0; p2 < 256; p2++) {
          p2elt = (p2 << jshift);
          table[index] = galois_shift_multiply(p1elt, p2elt, 32);
          index++;
        }
      }
    }
  }
  return 0;
}

int galois_split_w8_multiply(int x, int y)
{
  int i, j, a, b, accumulator, i8, j8;

  accumulator = 0;

  i8 = 0;
  for (i = 0; i < 4; i++) {
    a = (((x >> i8) & 255) << 8);
    j8 = 0;
    for (j = 0; j < 4; j++) {
      b = ((y >> j8) & 255);
      accumulator ^= galois_split_w8[i+j][a|b];
      j8 += 8;
    }
    i8 += 8;
  }
  return accumulator;
}

int galois_create_split_w16_tables(){
    /* dead code 
    int p1, p2, i, j, index, ishift, jshift;
    uint64_t *table;
    uint64_t p1elt, p2elt;

    if (galois_split_w16[0] != NULL) return 0;
    if (galois_create_log_tables(16) == -1) return -1;

    try {
        for (i = 0; i < 7; i++) {
            galois_split_w16[i] = new uint64_t [( 1 << 16 )];
        }
    } catch (bad_alloc &e) {
        for (i = 0; i < 7; i++) {
            if (galois_split_w16[i] != NULL) delete galois_split_w16[i];
            galois_split_w16[i] = NULL;
        }
            return -1;
    }

    for (i = 0; i < 4; i += 3) {
        ishift = i * 16;
        for (j = ((i == 0) ? 0 : 1) ; j < 4; j++) {
            jshift = j * 16;
            table = galois_split_w16[i+j];
            index = 0;
            for (p1 = 0; p1 < 65536; p1++) {
                p1elt = (p1 << ishift);
                for (p2 = 0; p2 < 65536; p2++) {
                    p2elt = (p2 << jshift);
                    table[index] = galois_shift_multiply_64(p1elt, p2elt);
                    index++;
                }
            }
        }
    }
    */
    return 0;
}


uint64_t galois_split_w16_multiply(uint64_t x, uint64_t y)
{
    /* dead code 
    int i, j, a, b, i8, j8;
    uint64_t accumulator = 0;

    if (galois_create_split_w16_tables() < 0){
         printf("Cannot create split w16 tables!\n");
    }

    printf("begin computing\n");
    i8 = 0;
    for (i = 0; i < 4; i++) {
        a = (((x >> i8) & 65535) << 16);
        j8 = 0;
        for (j = 0; j < 4; j++) {
            b = ((y >> j8) & 65535);
            accumulator ^= galois_split_w16[i+j][a|b];
            j8 += 16;
        }
        i8 += 16;
  }
  return accumulator;
  */
    return x+y; 
}


// Previously in reed_sol:
void galois_w04_region_multby_2(unsigned char *region, int nbytes)
{
	unsigned int *l1;
	unsigned int *ltop;
	unsigned char *ctop;
	unsigned int tmp, tmp2;

	if (prim04 == -1) {
		tmp = galois_single_multiply((1 << 3), 2, 4);
		prim04 = 0;
		while (tmp != 0) {
			prim04 |= tmp;
			tmp = (tmp << 4);
		}
		tmp = (1 << 4) - 2;
		mask04_1 = 0;
		while (tmp != 0) {
			mask04_1 |= tmp;
			tmp = (tmp << 4);
		}
		tmp = (1 << 3);
		mask04_2 = 0;
		while (tmp != 0) {
			mask04_2 |= tmp;
			tmp = (tmp << 4);
		}
	}
	ctop = region + nbytes;
	ltop = (unsigned int *) ctop;
	l1 = (unsigned int *) region;

	while (l1 < ltop) {
		tmp = ((*l1) << 1) & mask04_1;
		tmp2 = (*l1) & mask04_2;
		tmp2 = ((tmp2 << 1) - (tmp2 >> 3));
		*l1 = (tmp ^ (tmp2 & prim04));
		l1++;
	}
}

void galois_w04_region_multby_2_64(unsigned char *region, int nbytes)
{
	uint64_t *l1;
	uint64_t *ltop;
	unsigned char *ctop;
	uint64_t tmp, tmp2;

	if (prim04_64 == -1UL) {
		tmp = (uint64_t)galois_single_multiply((1 << 3), 2, 4);
		prim04_64 = 0UL;
		while (tmp != 0UL) {
			prim04_64 |= tmp;
			tmp = (tmp << 4UL);
		}
		tmp = (1UL << 4UL) - 2UL;
		mask04_1_64 = 0UL;
		while (tmp != 0UL) {
			mask04_1_64 |= tmp;
			tmp = (tmp << 4UL);
		}
		tmp = (1UL << 3UL);
		mask04_2_64 = 0UL;
		while (tmp != 0UL) {
			mask04_2_64 |= tmp;
			tmp = (tmp << 4UL);
		}
/*        printf("prim08_64 = %"PRIx64"\n", prim08_64);
        printf("mask08_1_64 = %"PRIx64"\n", mask08_1_64);
       printf("mask08_2_64 = %"PRIx64"\n", mask08_2_64);*/
	}
	ctop = region + nbytes;
	ltop = (uint64_t *) ctop;
	l1 = (uint64_t *) region;

	while (l1 < ltop) {
		tmp = ((*l1) << 1UL) & mask04_1_64;
		tmp2 = (*l1) & mask04_2_64;
		tmp2 = ((tmp2 << 1UL) - (tmp2 >> 3UL));
		*l1 = (tmp ^ (tmp2 & prim04_64));
		l1++;
	}
}

void galois_w08_region_multby_2(unsigned char *region, int nbytes)
{
	unsigned int *l1;
	unsigned int *ltop;
	unsigned char *ctop;
	unsigned int tmp, tmp2;

	if (prim08 == -1) {
		tmp = galois_single_multiply((1 << 7), 2, 8);
		prim08 = 0;
		while (tmp != 0) {
			prim08 |= tmp;
			tmp = (tmp << 8);
		}
		tmp = (1 << 8) - 2;
		mask08_1 = 0;
		while (tmp != 0) {
			mask08_1 |= tmp;
			tmp = (tmp << 8);
		}
		tmp = (1 << 7);
		mask08_2 = 0;
		while (tmp != 0) {
			mask08_2 |= tmp;
			tmp = (tmp << 8);
		}
	}
	ctop = region + nbytes;
	ltop = (unsigned int *) ctop;
	l1 = (unsigned int *) region;

	while (l1 < ltop) {
		tmp = ((*l1) << 1) & mask08_1;
		tmp2 = (*l1) & mask08_2;
		tmp2 = ((tmp2 << 1) - (tmp2 >> 7));
		*l1 = (tmp ^ (tmp2 & prim08));
		l1++;
	}
}

void galois_w08_region_multby_2_64(unsigned char *region, int nbytes)
{
	uint64_t *l1;
	uint64_t *ltop;
	unsigned char *ctop;
	uint64_t tmp, tmp2;

	if (prim08_64 == -1UL) {
		tmp = (uint64_t)galois_single_multiply((1 << 7), 2, 8);
		prim08_64 = 0UL;
		while (tmp != 0UL) {
			prim08_64 |= tmp;
			tmp = (tmp << 8UL);
		}
		tmp = (1UL << 8UL) - 2UL;
		mask08_1_64 = 0UL;
		while (tmp != 0UL) {
			mask08_1_64 |= tmp;
			tmp = (tmp << 8UL);
		}
		tmp = (1UL << 7UL);
		mask08_2_64 = 0UL;
		while (tmp != 0UL) {
			mask08_2_64 |= tmp;
			tmp = (tmp << 8UL);
		}
/*        printf("prim08_64 = %"PRIx64"\n", prim08_64);
        printf("mask08_1_64 = %"PRIx64"\n", mask08_1_64);
       printf("mask08_2_64 = %"PRIx64"\n", mask08_2_64);*/
	}
	ctop = region + nbytes;
	ltop = (uint64_t *) ctop;
	l1 = (uint64_t *) region;

	while (l1 < ltop) {
		tmp = ((*l1) << 1UL) & mask08_1_64;
		tmp2 = (*l1) & mask08_2_64;
		tmp2 = ((tmp2 << 1UL) - (tmp2 >> 7UL));
		*l1 = (tmp ^ (tmp2 & prim08_64));
		l1++;
	}
}

void galois_w16_region_multby_2(unsigned char *region, int nbytes)
{
	unsigned int *l1;
	unsigned int *ltop;
	unsigned char *ctop;
	unsigned int tmp, tmp2;

	if (prim16 == -1) {
		tmp = galois_single_multiply((1 << 15), 2, 16);
		prim16 = 0;
		while (tmp != 0) {
			prim16 |= tmp;
			tmp = (tmp << 16);
		}
		tmp = (1 << 16) - 2;
		mask16_1 = 0;
		while (tmp != 0) {
			mask16_1 |= tmp;
			tmp = (tmp << 16);
		}
		tmp = (1 << 15);
		mask16_2 = 0;
		while (tmp != 0) {
			mask16_2 |= tmp;
			tmp = (tmp << 16);
		}
	}

	ctop = region + nbytes;
	ltop = (unsigned int *) ctop;
	l1 = (unsigned int *) region;
	while (l1 < ltop) {
		tmp = ((*l1) << 1) & mask16_1;
		tmp2 = (*l1) & mask16_2;
		tmp2 = ((tmp2 << 1) - (tmp2 >> 15));
		*l1 = (tmp ^ (tmp2 & prim16));
		l1++;
	}
}

void galois_w16_region_multby_2_64(unsigned char *region, int nbytes)
{
	uint64_t *l1;
	uint64_t *ltop;
	unsigned char *ctop;
	uint64_t tmp, tmp2;

	if (prim16_64 == -1UL) {
		tmp = (uint64_t)galois_single_multiply((1 << 15), 2, 16);
		prim16_64 = 0UL;
		while (tmp != 0UL) {
			prim16_64 |= tmp;
			tmp = (tmp << 16UL);
		}
		tmp = (1UL << 16UL) - 2UL;
		mask16_1_64 = 0UL;
		while (tmp != 0UL) {
			mask16_1_64 |= tmp;
			tmp = (tmp << 16UL);
		}
		tmp = (1UL << 15UL);
		mask16_2_64 = 0UL;
		while (tmp != 0UL) {
			mask16_2_64 |= tmp;
			tmp = (tmp << 16UL);
		}
       /* printf("prim16_64 = %"PRIx64"\n", prim16_64);
        printf("mask16_1_64 = %"PRIx64"\n", mask16_1_64);
        printf("mask16_2_64 = %"PRIx64"\n", mask16_2_64); */
	}

	ctop = region + nbytes;
	ltop = (uint64_t *) ctop;
	l1 = (uint64_t *) region;
	while (l1 < ltop) {
		tmp = ((*l1) << 1UL) & mask16_1_64;
		tmp2 = (*l1) & mask16_2_64;
		tmp2 = ((tmp2 << 1UL) - (tmp2 >> 15UL));
		*l1 = (tmp ^ (tmp2 & prim16_64));
		l1++;
	}
}

void galois_w32_region_multby_2(unsigned char *region, int nbytes)
{
	int *l1;
	int *ltop;
	unsigned char *ctop;

	if (prim32 == -1) prim32 = galois_single_multiply((1 << 31), 2, 32);

	ctop = region + nbytes;
	ltop = (int *) ctop;
	l1 = (int *) region;

	while (l1 < ltop) {
		*l1 = ((*l1) << 1) ^ ((*l1 & 0x80000000) ? prim32 : 0);
		l1++;
	}
}

void galois_w32_region_multby_2_64(unsigned char *region, int nbytes)
{
	uint64_t *l1;
	uint64_t *ltop;
	unsigned char *ctop;
	uint64_t tmp, tmp2;

	if (prim32_64 == -1UL) {
		tmp = (uint64_t)galois_single_multiply((1 << 31), 2, 32);
		prim32_64 = 0UL;
		while (tmp != 0UL) {
			prim32_64 |= tmp;
			tmp = (tmp << 32UL);
		}
		tmp = (1UL << 32UL) - 2UL;
		mask32_1_64 = 0UL;
		while (tmp != 0UL) {
			mask32_1_64 |= tmp;
			tmp = (tmp << 32UL);
		}
		tmp = (1UL << 31UL);
		mask32_2_64 = 0UL;
		while (tmp != 0UL) {
			mask32_2_64 |= tmp;
			tmp = (tmp << 32UL);
		}
      /*  printf("prim32_64 = %"PRIx64"\n", prim32_64);
        printf("mask32_1_64 = %"PRIx64"\n", mask32_1_64);
        printf("mask32_2_64 = %"PRIx64"\n", mask32_2_64); */
	}

	ctop = region + nbytes;
	ltop = (uint64_t *) ctop;
	l1 = (uint64_t *) region;
	while (l1 < ltop) {
		tmp = ((*l1) << 1UL) & mask32_1_64;
		tmp2 = (*l1) & mask32_2_64;
		tmp2 = ((tmp2 << 1UL) - (tmp2 >> 31UL));
		*l1 = (tmp ^ (tmp2 & prim32_64));
		l1++;
	}
}

void galois_w64_region_multby_2(unsigned char *region, int nbytes)
{
	uint64_t *l1;
	uint64_t *ltop;
	unsigned char *ctop;

	ctop = region + nbytes;
	ltop = (uint64_t *) ctop;
	l1 = (uint64_t *) region;

	while (l1 < ltop) {
		*l1 = ((*l1) << 1) ^ ((*l1 & 0x8000000000000000) ? prim64 : 0);
		l1++;
	}
}
