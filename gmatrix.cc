#include "nc.h"
#include "galois.h"

/* Create an empty matrix */
GMatrix::GMatrix(){
    rr = 0;
    cc = 0;
}

/* Create an matrix , Set r, c and resize elems */
GMatrix::GMatrix(int r, int c, int w){
    rr = r;
    cc = c;
    ww = w;

    Resize_matrix();
}



/* Set a matrix */
int GMatrix::Make_from_list(int *p, int r, int c, int w){
    int i,j;
    int shift;

    rr = r; 
    cc = c;
    ww = w;
    ele8.clear();
    ele16.clear();
    ele32.clear();
    Resize_matrix();

    for(i = 0; i < r; i++){
        for(j = 0; j < c; j++){
            shift = i*c + j;
            Set(i, j, *(p+shift));
        }
    }
}

/* Set the matrix from a file */
int GMatrix::Make_from_file(FILE *fp, int rows, int cols, int w){
    int i, j;
    unsigned int elem;
    
    rr = rows;
    cc = cols;
    ww = w;
    ele8.clear();
    ele16.clear();
    ele32.clear();
    Resize_matrix();

    try{
        for(i = 0; i < rows; i++){
            for(j = 0; j < cols; j++){
                fscanf(fp, "%u", &elem);
                Set(i, j, elem);
            }
        }
    }catch(...){
        NOTE("Error when set the matrix from a file");
        return 0;
    }
    return 1;
}

int GMatrix::Make_from_string(string str, int rows, int cols, int w){
    int i, j;
    uint64_t ele;
    istringstream eles(str, istringstream::in);

    assert(str.length() == (rows*cols));
    rr = rows;
    cc = cols;
    ww = w;

    ele8.clear();
    ele16.clear();
    ele32.clear();
    Resize_matrix();

    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j++){
            eles>>ele;
            Set(i, j, ele);
        }
    }
    
}


/* Set values of previously allocated matrix to all 0s */
int GMatrix::Make_zero(int r, int c, int w){
    ele8.clear();
    ele16.clear();
    ele32.clear();
    this->rr = r;
    this->cc = c;
    this->ww = w;

    return Resize_matrix();
}

/* Make an identity matrix. */
int GMatrix::Make_identity(int r, int c, int w){
    int i;

    assert(r == c);
    ele8.clear();
    ele16.clear();
    ele32.clear();
    Resize_matrix();
    
    Make_zero(r, c, w);
    for( i = 0; i < r; i++ )
        Set(i, i, 1);
    
    return 1;
}

/* Make a vandermonde matrix */
int GMatrix::Make_vandermonde(int r, int c, int w){
    int i, j;
    int k;

    ele8.clear();
    ele16.clear();
    ele32.clear();
    Resize_matrix();
    this->rr = r;
    this->cc = c;
    this->ww = w;

    for(j = 0; j < c; j++){
        Set(0, j, 1);
    }
    for(i = 1; i < r; i++){
        k = 1;
        for(j = 0; j < c; j++){
            Set(i, j, k);
            k = galois_single_multiply(k, i+1, w);
        }
    }

    return 1;
}

/* Make a r(rows)*c(cols) size random GMatrix matrix */
int GMatrix::Make_random(int r, int c, int w){
    int i;
    int rand;
    char *pm;

    ele8.clear();
    ele16.clear();
    ele32.clear();
    Resize_matrix();
    rr = r;
    cc = c;
    ww = w;
    Resize_matrix();

    srand48(time(0));
    
    switch(ww){
        case 8:
            for(i = 0; i < rr*cc; i++){
                pm = (char *)&(ele8[i]);
                rand = lrand48();
                *(uint8_t *)pm = rand;
            }
            break;
        case 16:
            for(i = 0; i < rr*cc; i++){
                pm = (char *)&(ele16[i]);
                rand = lrand48();
                *(uint16_t *)pm = rand;
            }
            break;
        case 32:
            for(i = 0; i < rr*cc; i++){
                pm = (char *)&(ele32[i]);
                rand = lrand48();
                *(uint32_t *)pm = rand;
            }
            break;
        default:
            ERROR("Bad ww");
            
    }
}

/* The matrix is empty or not */
int GMatrix::Empty(){
    return (rr?-1:1);
}

/* Sets the r,c element of matrix to val */
void GMatrix::Set( int r, int c, uint64_t val){
    char *pm;

    switch(ww){
        case 8:
            pm = (char *)&(ele8[r*cc+c]);
            *((uint8_t *)pm) = val;
            break;
        case 16:
            pm = (char *)&(ele16[r*cc+c]);
            *((uint16_t *)pm) = val; 
            break;
        case 32:
            pm = (char *)&(ele32[r*cc+c]);
            *((uint32_t *)pm) = val;
            break;
        default:
            ERROR("Bad ww");
    }
}

/* Get the element from row r, colomn c */
uint64_t GMatrix::Get(int r, int c){
    switch(ww){
        case 8:
            return ele8[r*cc+c];
        case 16:
            return ele16[r*cc+c];
        case 32:
            return ele32[r*cc+c];
        default:
            ERROR("Bad ww");
    }
}

/* Resize the matrix */
int GMatrix::Resize_matrix(){
    try{
        switch(ww){
            case 8:
                ele8.resize(rr*cc, 0);
                break;
            case 16:
                ele16.resize(rr*cc, 0);
                break;
            case 32:
                ele32.resize(rr*cc, 0);
                break;
            default:
                ERROR("Bad ww");
        }
    }catch(...){
        return -1;
    }
    return 1;
}

/* Output the matrix */
void GMatrix::Print(int type){
    int i, j;
    int m[3] = {3, 5, 10};
    int n[3] = {2, 4, 8};

    switch(ww){
        case 8:
            if(1 == type){
                for(i = 0; i < rr ; i++){
                    for(j = 0; j < cc; j++){    
                            fprintf(stdout,"%0*u ", m[0], Get(i,j));
                    }
                    fprintf(stdout,"\n");
                }
            }
            else{
                for(i = 0; i < rr ; i++){
                    for(j = 0; j < cc; j++){
                        fprintf(stdout,"%0*x " ,n[0],Get(i,j));
                    }
                    fprintf(stdout,"\n");
                } 
            } 
            break;

        case 16:
            if(1 == type){
                for(i = 0; i < rr ; i++){
                    for(j = 0; j < cc; j++){
                            fprintf(stdout,"%0*u ", m[1], Get(i,j));
                    }
                    fprintf(stdout,"\n");
                }
            }
            else{
                for(i = 0; i < rr ; i++){
                    for(j = 0; j < cc; j++){
                        fprintf(stdout,"%0*x " ,n[1], Get(i,j));
                    }
                    fprintf(stdout,"\n");
                } 
            } 
            break;
        case 32:
            if(1 == type){
                for(i = 0; i < rr ; i++){
                    for(j = 0; j < cc; j++){
                            fprintf(stdout,"%0*u ", m[2], Get(i,j));
                    }
                    fprintf(stdout,"\n");
                }
            }
            else{
                for(i = 0; i < rr ; i++){
                    for(j = 0; j < cc; j++){
                        fprintf(stdout,"%0*x " ,n[2],Get(i,j));
                    }
                    fprintf(stdout,"\n");
                } 
            } 
            break;
        default:
            ERROR("Bad ww");
    }
}

/* Output the matrix as a long string*/
void GMatrix::Print_as_str(){
    int i,j;
    
    switch(ww){
        case 8:
            for(i = 0; i < rr; i++){
                for(j = 0; j < cc; j++){
                    fprintf(stdout,"%2x ", Get(i,j));
                }
            }
            break;
        case 16:
            for(i = 0; i < rr; i++){
                for(j = 0; j < cc; j++){
                    fprintf(stdout,"%4x ", Get(i,j));
                }
            }
            break;
        case 32:
            for(i = 0; i < rr; i++){
                for(j = 0; j < cc; j++){
                    fprintf(stdout,"%8x ", Get(i,j));
                }
            }
            break;
        default:
            ERROR("Bad ww");
    }
    fprintf(stdout,"\n");
}

/* overwrites the values of matrix with values of m */
int GMatrix::Copy(GMatrix *mat){
    rr = mat->rr;
    cc = mat->cc;
    ww = mat->ww;

    try{
        switch(ww){
            case 8:
                ele8 = mat->ele8;
                break;
            case 16:
                ele16 = mat->ele16;
                break;
            case 32:
                ele32 = mat->ele32;
                break;
            default:
                ERROR("Bad ww");
        }
    }catch(...){ return -1;}
    return 1;
}

/* Inverse the matrix */
void GMatrix::Inverse(){
    GMatrix mat_inv;
    int dim;
    int i, nzero_row, j;
    int temp, jtimes;

    assert(rr == cc);
    dim = rr;
    mat_inv.Make_identity(rr, cc, ww);

    for(i = 0; i < dim; i++){
        nzero_row = i;
        if(0 == Get(i,i)){
            do{
                ++nzero_row;
                if(nzero_row >= dim){
                    ERROR("Non-full rank matrix!");
                }
                temp = Get(nzero_row, i);
            }while((0 == temp)&&(nzero_row < dim));
            Swap_rows(i, nzero_row);
            mat_inv.Swap_rows(i, nzero_row);
        }
    
        for(j = 0; j < dim; j++){
            if(0 == Get(j,i))
                continue;
            if(j != i){
                jtimes = galois_single_divide(Get(j,i), Get(i,i), ww);
                Row_plus_irow(j, i, jtimes);
                mat_inv.Row_plus_irow(j, i, jtimes);
            }else{
                jtimes = galois_inverse(Get(i, i), ww);
                Row_to_irow(i , jtimes);
                mat_inv.Row_to_irow(i , jtimes);
            }
        }
    }

    switch(ww){
        case 8:
            ele8 = mat_inv.ele8;
            break;
        case 16:
            ele16 = mat_inv.ele16;
            break;
        case 32:
            ele32 = mat_inv.ele32;
            break;
    }
}


/* row r1 = row r1 + row r2*/
void GMatrix::Row_plus_row(int r1, int r2){
    int j;
    
    for(j = 0 ; j < cc ; j++){
        Set(r1, j, Get(r1, j) ^ Get(r2, j) );
    }
}

/* row r1 = row r1 + row r2 * prod */
void GMatrix::Row_plus_irow(int r1, int r2, int prod){
    int i;

    for(i = 0; i < cc; i++){
        Set(r1 , i , Get(r1, i)^(galois_single_multiply(Get(r2, i), prod, ww)) );
    }
}

/*row r = row r * prod */
void GMatrix::Row_to_irow(int row, int prod){
    int i;
    
    for(i = 0; i < cc; i++){
        Set(row, i, galois_single_multiply(Get(row, i), prod, ww));
    }
}

/* swap r1 and r2 */
void GMatrix::Swap_rows(int r1 , int r2){
    int tmp;
    int j;

    for(j = 0 ; j < cc ; j++){
        tmp = Get(r1,j);
        Set( r1 , j , Get(r2, j) );
        Set( r2 , j , tmp );
    }
}

/* swap c1 and c2 */
void GMatrix::Swap_cols(int c1 , int c2){
    int tmp;
    int j;

    for(j = 0 ; j < rr ; j++){
        tmp = Get(j,c1);
        Set( j , c1 , Get(j,c2) );
        Set( j , c2 , tmp);
    }
}

/* Delete row row */
void GMatrix::Del_row(int row){
    int i,j;
    
    assert(row < rr);
    
    if(row == (rr - 1));
    else{
        switch(ww){
            case 8:
                for(i = row; i < (rr-1); i++){
                    for(j = 0; j < cc; j++){
                        ele8[i*cc + j] = ele8[(i+1)*cc + j];
                    }
                }
                break;
            case 16:
                for(i = row; i < (rr-1); i++){
                    for(j = 0; j < cc; j++){
                        ele16[i*cc + j] = ele16[(i+1)*cc + j];
                    }
                }
                break;
            case 32:
                for(i = row; i < (rr-1); i++){
                    for(j = 0; j < cc; j++){
                        ele32[i*cc + j] = ele32[(i+1)*cc + j];
                    }
                }
                break;
        }
    }
    --rr;
    Resize_matrix();
}

/* Delete lens rows from the begin*/
void GMatrix::Del_rows(int begin, int len){
    int i;
    int rows;

    assert((begin >= 0)&&(begin < rr));
    assert(len > 0);

    for(i = 0; i <= len; i++){
        Del_row(begin);
    }
}

/* Delete column col */
void GMatrix::Del_col(int col){
    int i, j;
    int shift;

    assert(col >= 0);
    assert(col < cc);

    shift = 0;
    --cc;
    
    switch(ww){
        case 8:
            for(i = 0; i < rr; i++){
                for(j = 0; j < cc; j++){
                    if(col == j) ++shift;
                    ele8[i*cc+j] = ele8[i*cc+j+shift];
                }
                if(col == j) ++shift;
            }
            break;
        case 16:
            for(i = 0; i < rr; i++){
                for(j = 0; j < cc; j++){
                    if(col == j) ++shift;
                    ele16[i*cc+j] = ele16[i*cc+j+shift];
                }
                if(col == j) ++shift;
            }

            break;
        case 32:
            for(i = 0; i < rr; i++){
                for(j = 0; j < cc; j++){
                    if(col == j) ++shift;
                    ele32[i*cc+j] = ele32[i*cc+j+shift];
                }
                if(col == j) ++shift;
            }
            break;
    }

    Resize_matrix();
}

/* Delete len cols from the begin col */
void GMatrix::Del_cols(int begin, int len){
    int i, j;
    int shift;
     
    assert(begin >= 0);
    assert((begin+len) < cc);
    assert(len > 0);

    shift = 0; 
    cc = cc - len;

    for(i = 0; i < rr; i++){
        for(j = 0; j < cc; j++){
            if(j == begin){
                shift = shift + len;
            }
            Set(i, j, Get(i,j+shift));
        }
        if(j == begin){
            shift =  shift + len;
        }
    }

    Resize_matrix();
}

/* Insert a matrix before the from-th row */
void GMatrix::Insert_matrix(GMatrix mat_ins, int from){
    int rest_rows;
    int raw_rows;
    int i, j;

    assert(from >= 0);
    assert(from < rr);
    assert(mat_ins.cc == cc);

    raw_rows = rr;
    rr = rr + mat_ins.rr;
    Resize_matrix();
    
    /* How many rows have to move */
    rest_rows = raw_rows - from;
    
    /* Have to move from the last row! */
    for(i = rest_rows-1; i >= 0; i--){
        for(j = 0; j < cc; j++){
            Set(from + mat_ins.rr + i, j, Get(from+i, j));
        }
    }

    for(i = 0; i < mat_ins.rr; i++){
        for(j = 0; j < cc; j++){
            Set(from+i, j, mat_ins.Get(i, j));
        }
    }
}

/* Insert len rows of a matrix before the from-th row */
void GMatrix::Insert_matrix(GMatrix mat_ins, int from, int len){
    int rest_rows;
    int raw_rows;
    int i, j;

    assert(from >= 0);
    assert(from < rr);
    assert(len <= mat_ins.rr);
    assert(mat_ins.cc == cc);

    raw_rows = rr;
    rr = rr + len;
    Resize_matrix();
    
    /* How many rows have to move */
    rest_rows = raw_rows - from;
    
    /* Have to move from the last row! */
    for(i = rest_rows-1; i >= 0; i--){
        for(j = 0; j < cc; j++){
            Set(from + len + i, j, Get(from+i, j));
        }
    }

    for(i = 0; i < len; i++){
        for(j = 0; j < cc; j++){
            Set(from+i, j, mat_ins.Get(i, j));
        }
    }
}

/* Append a matrix after the matrix
 * an addition of Insert_matrix() */
void GMatrix::Append_matrix(GMatrix mat_app){
    int i, j;
    int end;

    assert(mat_app.cc == cc);
    
    end = rr;
    rr = rr + mat_app.rr;
    Resize_matrix();

    for(i = 0; i < mat_app.rr; i++){
        for(j = 0; j < cc; j++){
            Set(end + i, j, mat_app.Get(i, j));
        }
    }
}

/* Append a matrix from the begin after the matrix*/
void GMatrix::Append_matrix(GMatrix mat_app, int begin, int len){
    int i, j;
    int end;

    assert(mat_app.cc == cc);
    assert(begin >= 0);
    assert(mat_app.rr >= (begin+len));
    
    end = rr;
    rr = rr + len;
    Resize_matrix();

    for(i = 0; i < len; i++){
        for(j = 0; j < cc; j++){
            Set(end + i, j, mat_app.Get(begin+i,j));
        }
    }
}

/* Replace from the from-th row of the matrix with mat */
void GMatrix::Replace_matrix(GMatrix mat, int begin, int len){
    int i, j;

    assert(cc == mat.cc);
    assert(mat.rr >= len);
    assert((begin + len) <= rr );

    for(i = 0; i < len; i++){
        for(j = 0; j < mat.cc; j++){
            Set(begin+i, j, mat.Get(i, j));
        }
    }
}

/* Replace the matrix with mat from the from-th row */
void GMatrix::Replace_matrix(GMatrix mat, int begin){
    int i, j;

    assert(cc == mat.cc);
    assert((begin + mat.rr) <= rr);

    for(i = 0; i < mat.rr; i++){
        for(j = 0; j < mat.cc; j++){
            Set(begin + i, j, mat.Get(i, j));
        }
    }
}

/* Wipe the matrix all value with value */
void GMatrix::Wipe_matrix(int value){
    int i,j;
    
    for(i = 0; i < rr; i++){
        for(j = 0; j < cc; j++)
            Set(i, j, value);
    }
}

/* Wipe len rows of the matrix from begin with value */
void GMatrix::Wipe_matrix(int begin, int len, int value){
    int i, j;
    
    switch(ww){
        case 8:
            for(i = 0; i < len; i++){
                for(j = 0; j < cc; j++){
                    ele8[(begin+i)*cc + j] = (uint8_t)value;
                }
            }
            break;
        case 16:
            for(i = 0; i < len; i++){
                for(j = 0; j < cc; j++){
                    ele16[(begin+i)*cc + j] = (uint16_t)value;
                }
            }
            break;
        case 32:
            for(i = 0; i < len; i++){
                for(j = 0; j < cc; j++){
                    ele32[(begin+i)*cc + j] = (uint32_t)value;
                }
            }
            break;
    }
}

/***********************
 * non-class functions *
 ***********************/

/* return matrix m1 * m2*/
GMatrix Prod( GMatrix m1 , GMatrix m2 ){
    GMatrix ret;
    int i, j, k;
    int r, c, w;
    
    assert(m1.cc == m2.rr);
    assert(m1.ww == m2.ww);
    
    ret.Make_zero(m1.rr , m2.cc, m1.ww);
    w = ret.ww;
    r = m1.rr;
    c = m2.cc;
    
    for(i = 0 ; i < r ; i++){
        for(j = 0 ; j < c; j++){
            for(k = 0 ; k < m1.cc ; k++){
                 ret.Set(i, j,((ret.Get(i,j))^(galois_single_multiply(m1.Get(i,k), m2.Get(k,j), w))));
            }
        }
    }

    return ret;
}

/* The row-th row of mat multiply the p_src 
 * and store in the p_des
 * the length of the string p_src point to is length */
int Prod(GMatrix mat, int row, unsigned char *p_des, unsigned char *p_src, int len){
    int i;
    int round;

    assert(0 == len%(mat.cc));
    assert(mat.rr >= row);

    round= len/mat.cc;
    for(i = 0; i < mat.cc; i++){       
        galois_w08_region_multiply(p_src+i*round, mat.Get(row,i), round, p_des, 1);
    }
}


/* Matrix m3 = m1 * m2 */
int Prod( GMatrix *m3 , GMatrix *m1 , GMatrix *m2){
    int i,j,k;
    int r, c, w;

    assert(m1->cc == m2->rr);
    assert(m1->ww == m2->ww);
    
    m3->Make_zero(m1->rr , m2->cc, m1->ww);
    w = m3->ww;
    r = m1->rr;
    c = m2->cc;

    for(i = 0 ; i < r ; i++){
        for(j = 0 ; j < c ; j++){
            for(k = 0 ; k < m1->cc ; k++){
    /* m3->Set(i, j,((m3->Get(i,j))^(galois(m1->Get(i,k), m2->Get(k,j), w)))) */
                switch(w){
                    case 8:
                        m3->ele8[i*c+j] = (m3->ele8[i*c+j]) ^ (galois_single_multiply(m1->Get(i,k), m2->Get(k,j), w));
                        break;
                    case 16:
                        m3->ele16[i*c+j] = (m3->ele16[i*c+j]) ^ (galois_single_multiply(m1->Get(i,k), m2->Get(k,j), w));
                        break;
                    case 32:
                        m3->ele32[i*c+j] = (m3->ele32[i*c+j]) ^ (galois_single_multiply(m1->Get(i,k), m2->Get(k,j), w));
                        break;
                }
            }
        }
    }
}


/* Add m1 and m2 */
GMatrix Add(GMatrix m1, GMatrix m2){
    GMatrix ret;
    int i, j;

    assert(m1.rr == m2.rr);
    assert(m1.cc == m2.cc);
    assert(m1.ww == m2.ww);

    ret.Make_zero(ret.rr, ret.cc, ret.ww);
    
    for(i = 0; i < m1.rr; i++){
        for(j = 0; j < m1.cc; j++){
            ret.Set(i, j, ((m1.Get(i, j))^(m2.Get(i, j))));
        }
    }

    return ret;
}

/* m3 = m2 + m1*/
void Add(GMatrix &m3, GMatrix &m1, GMatrix &m2){
    int i, j;
    int w;
    int c;

    assert((m1.rr == m2.rr)&&(m3.rr == m1.rr));
    assert((m1.cc == m2.cc)&&(m3.cc == m3.cc));
    assert((m1.ww == m2.ww)&&(m3.ww == m3.ww));

    w = m1.ww;
    c = m1.cc;
    for(i = 0; i < m1.rr; i++){
        for(j = 0; j < m1.cc; j++){
            /* Equal to m3.Set(i, j, (m1.Get(i, j)^(m2.Get(i, j))))*/
            switch(w){
                case 8:
                    m3.ele8[i*c +j] = (m1.ele8[i*c+j])^(m2.ele8[i*c+j]);
                    break;
                case 16:
                    m3.ele8[i*c +j] = (m1.ele8[i*c+j])^(m2.ele8[i*c+j]);
                    break;
                case 32:
                    m3.ele8[i*c +j] = (m1.ele8[i*c+j])^(m2.ele8[i*c+j]);
                    break;
            }
        }
    }
}

/* Return the rank of the matrix */
int Rank(GMatrix mat){
    GMatrix mat_cpy;
    int nz_row, nz_col;
    int i, j;
    int temp, jtimes;

    Copy(mat_cpy,mat);
    
    nz_col= 0;
    for(i = 0; i < mat_cpy.rr; i++){
        nz_row = i;
        nz_col = (nz_row > nz_col)?nz_row:nz_col;
        if(i == (mat_cpy.cc-1)){
            if(mat_cpy.Get(nz_row, nz_col) != 0){
                return mat_cpy.cc;
            }
            else{
                return (mat_cpy.cc -1);
            }
        }
        
        while(0 == mat_cpy.Get(i, nz_col)){
            nz_row = i;
            do{
                ++nz_row;
                if(nz_row >= mat_cpy.rr){
                    break;
                }
                temp = mat_cpy.Get(nz_row, nz_col);
            }while((0 == temp)&&(nz_row < mat_cpy.rr));

            if(nz_row < mat_cpy.rr){
                mat_cpy.Swap_rows(i, nz_row);
                break;
            }
            ++nz_col;
            if(nz_col >= mat_cpy.cc){
                return i;
            }
        }
        for(j = i+1; j < mat_cpy.rr; j++){
            if(mat_cpy.Get(j, nz_col) == 0)
                continue;
            jtimes = galois_single_divide(mat_cpy.Get(j,nz_col), mat_cpy.Get(i,nz_col), mat_cpy.ww);
            mat_cpy.Row_plus_irow(j, i, jtimes);
        }
    }

    return mat.rr;
}



/* Is full rank or not?
 * rr of full-rank matrix equals non-zero rows in the matrix
 * return 1 if it's full rank matrix, otherwise return 0 */
int Is_full(GMatrix mat){
    GMatrix mat_cpy;
    int nz_row, nz_col;     /* equal nzero_row in Inverse() */
    int i, j;
    uint32_t temp, jtimes;

    if(mat.rr > mat.cc){
        return -1;
    }
    if(mat.rr <= 0){
        return -1;
    }
    if(mat.cc <= 0){
        return -1;
    }

    mat_cpy = mat;
    
    nz_col= 0;
    for(i = 0; i < mat_cpy.rr; i++){
        nz_row = i;
        nz_col = (nz_row > nz_col)?nz_row:nz_col;
        if(i == (mat_cpy.cc-1)){
            if(mat_cpy.Get(nz_row, nz_col) != 0){
                return 1;
            }
            else{
                return -1;
            }
        }
        
        while(0 == mat_cpy.Get(i, nz_col)){
            nz_row = i;
            do{
                ++nz_row;
                if(nz_row >= mat_cpy.rr){
                    break;
                }
                temp = mat_cpy.Get(nz_row, nz_col);
            }while((0 == temp)&&(nz_row < mat_cpy.rr));

            if(nz_row < mat_cpy.rr){
                mat_cpy.Swap_rows(i, nz_row);
                break;
            }
            ++nz_col;
            if(nz_col >= mat_cpy.cc){
                return -1;
            }
        }

        for(j = i+1; j < mat_cpy.rr; j++){
            if(0 == mat_cpy.Get(j, nz_col))
                continue;
            jtimes = galois_single_divide(mat_cpy.Get(j,nz_col), mat_cpy.Get(i,nz_col), mat_cpy.ww);
            mat_cpy.Row_plus_irow(j, i, jtimes);
        }
    }
    return 1;
}    

/* Inversion of a matrix */
int Inverse( GMatrix &dst_mat , GMatrix &src_mat){
    dst_mat = Inverse(src_mat);
}

GMatrix Inverse(GMatrix mat){
    GMatrix mat_inv;
    GMatrix mat_cpy;
    int dim;
    int i, nzero_row, j;
    uint32_t temp, jtimes;

    assert(mat.rr == mat.cc);
    
    dim = mat.rr;
    /* mat_cpy = mat;*/
    Copy(mat_cpy , mat);
    
    mat_inv.Make_identity(mat.rr, mat.cc, mat_cpy.ww);

    /* from column 0 to column dim-1 */
    for(i = 0; i < dim; i++){
        nzero_row = i;

        /* matrix(i, i) == 1  add the nzero_row th row 
         * which elems[nzero_row*dim +i] != 0 to the ith row*/
        if(0 == mat_cpy.Get(i,i)){
            do{
                ++nzero_row; 
                if(nzero_row >= dim){
                    mat_cpy.Print();
                    ERROR("Non-full rank matrix!");
                }
                temp = mat_cpy.Get(nzero_row,i);
            }while((0 == temp)&&(nzero_row < dim));
            mat_cpy.Swap_rows(i, nzero_row);
            mat_inv.Swap_rows(i, nzero_row);
        }

        /* matrix(i, i) != 0 now */
        for(j = 0; j < dim; j++){
            if(0 == mat_cpy.Get(j,i))   continue;
            if(j != i){
                jtimes = galois_single_divide(mat_cpy.Get(j,i), mat_cpy.Get(i,i), mat_cpy.ww);
                mat_cpy.Row_plus_irow(j, i, jtimes);
                mat_inv.Row_plus_irow(j, i, jtimes);
            }
            else{
                jtimes = galois_inverse(mat_cpy.Get(i,i), mat_cpy.ww);
                mat_cpy.Row_to_irow(i , jtimes);
                mat_inv.Row_to_irow(i , jtimes);
            }
            
        }
    }
    return mat_inv;
}

/* Transpose the matrix along the diagonal */
GMatrix Transpose(GMatrix mat){
    GMatrix ret;
    int i,j;

    ret.Make_zero(mat.cc, mat.rr, mat.ww);
    
    switch(mat.ww){
        case 8:
            for(i = 0; i < ret.rr; i++){
                for(j = 0; j < ret.cc; j++){
                    ret.ele8[i*ret.cc +j] = mat.ele8[(j*mat.cc + i)];
                }
            }
            break;
        case 16:
            for(i = 0; i < ret.rr; i++){
                for(j = 0; j < ret.cc; j++){
                    ret.ele16[i*ret.cc +j] = mat.ele16[(j*mat.cc + i)];
                }
            }
            break;
        case 32:
            for(i = 0; i < ret.rr; i++){
                for(j = 0; j < ret.cc; j++){
                    ret.ele32[i*ret.cc +j] = mat.ele32[(j*mat.cc + i)];
                }
            }
            break;
    }

    return ret;
}


/* Copy matrix src_mat -> dst_mat*/
void Copy( GMatrix &dst_mat , GMatrix &src_mat ){
    int ww;
    
    dst_mat.rr = src_mat.rr;
    dst_mat.cc = src_mat.cc;
    dst_mat.ww = src_mat.ww;
    
    ww = dst_mat.ww;
    switch(ww){
        case 8:
            dst_mat.ele8 = src_mat.ele8;
            break;
        case 16:
            dst_mat.ele16 = src_mat.ele16;
            break;
        case 32:
            dst_mat.ele32 = src_mat.ele32;
            break;
    }
}

/* Copy matrix */
void Copy(GMatrix *dst_mat, GMatrix *src_mat){
    assert(dst_mat->ww == src_mat->ww);

    dst_mat->rr = src_mat->rr;
    dst_mat->cc = src_mat->cc;
    dst_mat->ww = src_mat->ww;

    switch(dst_mat->ww){
        case 8:
            dst_mat->ele8.resize(((dst_mat->rr)*(dst_mat->cc)),0);
            break;
        case 16:
            dst_mat->ele16.resize(((dst_mat->rr)*(dst_mat->cc)),0);
            break;
        case 32:
            dst_mat->ele32.resize(((dst_mat->rr)*(dst_mat->cc)),0);
            break;
    }
}

/* Draw a matrix with len rows from begin-th row of the matrix  */
GMatrix Slice_matrix(GMatrix mat, int begin, int len){
    GMatrix ret;
    int i, j;
    int col;

    assert(begin >= 0);
    assert(len > 0);
    assert(begin + len <= mat.rr);
    
    ret.rr = len;
    ret.cc = mat.cc;
    ret.ww = mat.ww;
    col = mat.cc;
    
    switch(ret.ww){
        case 8:
            ret.ele8.resize((ret.rr)*(ret.cc), 0);
            for(i = 0; i < col*len; i++)
                ret.ele8[i] = mat.ele8[i + begin*col];
            break;
        case 16:
            ret.ele16.resize((ret.rr)*(ret.cc), 0);
            for(i = 0; i < col*len; i++)
                ret.ele16[i] = mat.ele16[i + begin*col];
            break;
        case 32:
            ret.ele32.resize((ret.rr)*(ret.cc), 0);
            for(i = 0; i < col*len; i++)
                ret.ele32[i] = mat.ele32[i + begin*col];
            break;
    }
    
    return ret;
}