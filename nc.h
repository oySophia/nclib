#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <sys/time.h>
#include "galois.h"

#define PRT_STR(ss) {fprintf(stdout, "== %s == \n",ss);}
#define NOTE(ss) {fprintf(stdout, "=== %s === \n",ss);}
#define ERROR(ss) {fprintf(stdout, "## %s ##\n",ss);exit(0);}
#define PRT_RETURN {fprintf(stdout,"\n");}
#define DLLEXPORT extern "C"

#define  ntime struct timeval

int gettimeofday(struct timeval *tv,struct timezone *tz);

using namespace std;

/*****************
 ** GMatrix.cc **
 *****************/
class GMatrix{

    public:
        vector <uint8_t> ele8;
        vector <uint16_t> ele16;
        vector <uint32_t> ele32;
        int rr;
        int cc;
        int ww;

        /* Create the matrix with/without the numbers of    *
         * rows, colomns and wordsize of each element       */
        GMatrix();
        GMatrix(int r, int c, int w);
        
        /* Set the matrix from a string */
        int Make_from_list(int *p, int r, int c, int w);
        int Make_from_string(string str, int r, int c, int w);

        /* Set the matrix from a file */
        int Make_from_file(FILE *fp, int rows, int cols, int w);

        /* Set the all zero matrix or identity matrix       *
         * if success, return 1 , or false return 0         */
        int Make_zero(int r, int c, int w);
        int Make_identity(int r, int c, int w);
        int Make_vandermonde(int r, int c, int w);
        int Make_random(int r, int c, int w);

        /* 1:Empyt -1:Not empty */
        int Empty();

        /* Set/Get the value of row c , colomn c */
        void Set(int r, int c, uint64_t val);
        uint64_t Get(int r, int c);

        /* overwrites the values of matrix with values of m */
        int Copy(GMatrix *mat);
        
        /* Print the value of the matrix to stdout */
        void Print(int type = 1);

        /* Print the value of the matrix as a string */
        void Print_as_str();

        /* Inverse the matrix */
        void Inverse();
        
        /* Is full-rank matrix or not? yes:1 ; no:0*/
        int Is_full();

        /* swap r1 and r2 */
        void Swap_rows(int r1 , int r2);

        /* swap c1 and c2 */
        void Swap_cols(int c1 , int c2);

        /* row r1 = row r1 + row r2*/
        void Row_plus_row(int r1 , int r2);

        /* row r1 = row r1 + row r2 * prod */
        void Row_plus_irow(int r1 , int r2 , int prod);

        /* row r = row r * prod*/
        void Row_to_irow(int r, int prod);
        
        /* Delete row */
        void Del_row(int row);

        /* Delete len rows from the begin row*/
        void Del_rows(int begin, int len);

        /* Delete column col*/
        void Del_col(int col);

        /* Delete len cols from the begin */
        void Del_cols(int begin, int len);

        /* Insert a matrix before the from-th row */
        void Insert_matrix(GMatrix mat_ins, int from); 
        void Insert_matrix(GMatrix mat_ins, int from, int len); 

        /* Append a matrix mat_app after the matrix */
        void Append_matrix(GMatrix mat_app);

        /* Append a matrix from the begin-th row after the matrix*/
        void Append_matrix(GMatrix mat_app, int begin, int len);

        /* Replace from the from-th row of the matrix with mat */
        void Replace_matrix(GMatrix mat, int begin, int len);

        /* Replace the matrix with mat from the from-th row */
        void Replace_matrix(GMatrix mat, int begin);

        /* Wipe the matrix all value with value */
        void Wipe_matrix(int value);
        
        /* Wipe length rows of the matrix from begin with value */
        void Wipe_matrix(int begin, int length, int value);

    private:
        /* Resize the size of elems, and set elements */
        int Resize_matrix();
};

/* Return matrix m1 * m2 */
GMatrix Prod(GMatrix m1 , GMatrix m2);

/* The row-th row of mat multiply the p_src 
 * and store in the p_des
 * the length of the string p_src point to is length */
int Prod(GMatrix mat, int row, unsigned char *p_des, unsigned char *p_src, int len);

/* matrix m1*unsigned char [] */
GMatrix Prod(GMatrix &m1 , unsigned char * m2);

/* Matrix m3 = m1 * m2 */
int Prod(GMatrix *m3 , GMatrix *m1 , GMatrix *m2);

/* Add m1 and m2 */
GMatrix Add( GMatrix m1, GMatrix m2);

/* m3 = m2 + m1 */
void Add(GMatrix &m3, GMatrix &m1, GMatrix &m2);

/* Copy dst_mat from src_mat */
void Copy(GMatrix &dst_mat, GMatrix &src_mat);
        
/* Return the rank of a matrix */
int Rank(GMatrix mat);

/* Is full-rank matrix or not? yes:1 ; no:0 */
int Is_full(GMatrix mat);

/* Inverse the matrix m and return it */
GMatrix Inverse(GMatrix mat);
int Inverse(GMatrix &dst_mat, GMatrix &src_mat);

/* Transpose the matrix along the diagonal */
GMatrix Transpose(GMatrix mat);

/* Generate a r(rows)*c(cols) size GMatrix matrix */
/* GMatrix Random_GMatrix(int r, int c); */

/* Drag a matrix from the matrix with rows from begin to end */
GMatrix Slice_matrix(GMatrix mat, int begin, int len); 


/**************
 ** utils.cc **
 **************/

/* Generate a 0~num number */
DLLEXPORT int NC_random(int i);

/* Return the time interval */
double dt_s(struct timeval end,struct timeval start);

double dt_ms(struct timeval end,struct timeval start);

double dt_us(struct timeval end,struct timeval start);

/* Encode/Decode the data using GMatrix matrix         *
 * mat is the Encode/Decode matrix                      *
 * p_des store the data after Encode/Decode             *
 * p_src store the data before Encode/Decode            *
 * length represent the length of the data to deal with *
 * p_des and p_src must be long-word aligned            */
int NC_code(GMatrix mat, unsigned char *p_des, unsigned char *p_src, int length);

DLLEXPORT int NC_code_py(char * mat_c_p, int r, int c, int w, char *p_des, char *p_src, int length);

/* Encode a file using mat_en matrix                    *
 * fp_src points to the file to be encoded              *
 * fpp_des point to the files where encoded data store  */
int NC_encode_file(GMatrix mat_en, FILE *fp_src, FILE **fpp_des);

/* Decode files to a file                               *
 * fpp_src point to the files to be decoded             *
 * fp_des point to the file where decoded data store    */
int NC_decode_file(GMatrix mat_de, FILE **fpp_src, FILE *fp_des);

/* NK_property checks the system combining the new node *
 * satisfing (n,k) property, this is, k out of n nodes  *
 * can reconstruct the original data                    *
 * piece : how many rows of matrix mat in a node        */
int NK_property(GMatrix mat, int piece, int k);