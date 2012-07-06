#include <stdio.h>
#include <math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_eigen.h>
#include <assert.h>

#define DEBUG_SCF
//#define DEBUG_s_root

void Load();
gsl_matrix* S_i_root(double *S, int n);
void matrix_output(gsl_matrix *m, int n, char *msg);
void vector_output(gsl_vector *v, int n, char *msg);
gsl_matrix *Fock(double h[][4], double e2_int[][4][4][4], double density[][4], int n);
void density(double Density[][4], gsl_matrix* coef, int n);
void scf(gsl_matrix *f, const gsl_matrix *s_root, int n);

int main(int argc, char** argv)
{
    Load();
    return 0;
}

void Load()
{
    int n = 4;
    int i, j, k, l;
    double e2[n][n][n][n];
    FILE *f;

    char *file_name = "2e";
    double S[] = {
                    0.100000000E+01, 0.658291970E+00, 0.460875162E+00, 0.511249073E+00,
                    0.658291970E+00, 0.100000000E+01, 0.511249073E+00, 0.856364449E+00,
                    0.460875162E+00, 0.511249073E+00, 0.100000000E+01, 0.658291970E+00,
                    0.511249073E+00, 0.856364449E+00, 0.658291970E+00, 0.100000000E+01
                   };
    double HH[][4] = {
    {-0.965997649E+00,-0.923907639E+00,-0.818582204E+00,-0.783996285E+00},
    {-0.923907639E+00,-0.928703434E+00,-0.783996285E+00,-0.857664629E+00},
    {-0.818582204E+00,-0.783996285E+00,-0.965997649E+00,-0.923907639E+00},
    {-0.783996285E+00,-0.857664629E+00,-0.923907639E+00,-0.928703434E+00}
                };
    double Density[4][4];

    char *file_coeff = "coeff";
    gsl_matrix *coeff  = gsl_matrix_alloc(n, n);

    //读入系数矩阵
    f = fopen(file_coeff, "r");
    gsl_matrix_fscanf(f, coeff);
    fclose(f);
    //matrix_output(coeff, n, "系数矩阵:\n");

    //计算密度矩阵
    density(Density, coeff, n);

    // 读入双电子积分
    f = fopen(file_name, "r");
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            for (k = 0; k < n; k++) {
                for (l = 0; l < n; l++) {
                    //fscanf(f, "%lf", &e2[l][j][k][i]);
                    fscanf(f, "%lf", &e2[i][j][k][l]);
                }
            }
        }
    }
    fclose(f);
#undef NDEBUG
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            for (k = 0; k < n; k++) {
                for (l = 0; l < n; l++) {
                    assert(fabs(e2[i][j][k][l] - e2[i][j][l][k]) < 1.0E-6);
                    assert(fabs(e2[i][j][k][l] - e2[j][i][k][l]) < 1.0E-6);
                    assert(fabs(e2[i][j][k][l] - e2[k][l][i][j]) < 1.0E-6);
                    assert(fabs(e2[i][j][k][l] - e2[j][i][l][k]) < 1.0E-6);
                }
            }
        }
    }


    // 计算FOCK矩阵
    gsl_matrix *F = Fock(HH, e2, Density, n);
    matrix_output(F, n, "FOCK矩阵为:\n");
    gsl_matrix *s = S_i_root(S, n);
    scf(F, s, n);
}
void density(double Density[][4], gsl_matrix* coef, int n)
{
    int i, j, k;
    printf("密度矩阵为:\n");
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            Density[i][j] = 0;
            for (k = 0; k < 1; k++) {
                //Density[i][j] += 2 * coef[i][k] * coef[j][k];
                Density[i][j] += 2 * gsl_matrix_get(coef, i, k) * gsl_matrix_get(coef, j, k);
            }
            printf("%20.10lf", Density[i][j]);
        }
        printf("\n");
    }
}

void scf(gsl_matrix *f, const gsl_matrix *s_root, int n)
{
    gsl_matrix *eigVector = gsl_matrix_alloc(n, n);
    gsl_eigen_symmv_workspace *w = gsl_eigen_symmv_alloc(2*n);
    gsl_vector *eigValue = gsl_vector_alloc(n);
    
    //gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, s_root, f, 1.0, ft);
    //gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, ft, s_root, 1.0, ftt);

    //求本征矢量和本征值
    //gsl_eigen_symm(b, dialg_S, w);
    gsl_eigen_symmv(f, eigValue, eigVector, w);
#ifdef DEBUG_SCF
    vector_output(eigValue, n, "FOCK本征值为：\n");
    matrix_output(eigVector, n,  "FOCK本征矢量为:\n");
#endif

    gsl_matrix *c = gsl_matrix_calloc(n, n);

    
    //gsl_matrix s_root_invers = gsl_matrix_alloc(n, n);
    //gsl_matrix *inverse = gsl_matrix_alloc(n, n);
    //gsl_permutation *permutation = gsl_permutation_alloc(n);
    //gsl_matrix_memcpy(s_root_invers, s_root);

    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, s_root, eigVector, 1.0, c);
    matrix_output(c, n,  "新的轨道系数为:\n");


}

// 计算Fock矩阵
gsl_matrix *Fock(double h[][4], double e2_int[][4][4][4], double density[][4], int n)
{
    int i, j, k, l;
    double tmp = 0;
    gsl_matrix *m = gsl_matrix_alloc(n,n);

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            tmp = 0;
            for (k = 0; k < n; k++) {
                for (l = 0; l < n; l++)
                    tmp += ((2*e2_int[i][j][k][l] - e2_int[i][l][k][j]) / 4 * density[l][k]);
            }
            gsl_matrix_set(m, i, j, tmp + h[i][j]);
        }
    }
    return m;
}

// 此函数用于计算重叠矩阵逆阵的平方根
/*
 *  P * A * P^(-1) = D
 *
 *  sqrt(A) = P^(-1) * sqrt(D) * P
 *
 */
gsl_matrix* S_i_root(double *S, int n)
{

    gsl_matrix_view gv = gsl_matrix_view_array(S, n, n);
    gsl_matrix *a = gsl_matrix_alloc_from_matrix(&gv.matrix,0, 0, n, n);
    gsl_matrix *b = gsl_matrix_alloc(n, n);
    gsl_matrix *p = gsl_matrix_alloc(n, n);
    
    gsl_matrix_memcpy(b, a);
    gsl_matrix_free(a);
#ifdef DEBUG_s_root
    matrix_output(b, n, "初始重叠矩阵为:\n");
#endif
    gsl_eigen_symmv_workspace *w = gsl_eigen_symmv_alloc(2*n);
    gsl_vector *dialg_S = gsl_vector_alloc(n);

    //求本征矢量和本征值
    //gsl_eigen_symm(b, dialg_S, w);
    gsl_eigen_symmv(b, dialg_S, p, w);
#ifdef DEBUG_s_root
    vector_output(dialg_S, n, "重叠矩阵本征值为：\n");
    matrix_output(p, n,  "重叠矩阵本征矢量为:\n");
#endif

    // 将本征值开方
    int i;
    gsl_matrix *s_root = gsl_matrix_calloc(n, n);
    for (i = 0; i < n; i++) {
        gsl_matrix_set(s_root, i, i, 1/sqrt(dialg_S->data[i]));
        //gsl_matrix_set(s_root, i, i, dialg_S->data[i]);
    }
    //matrix_output(s_root, n, "the root of S^-1:\n");


    // 利用LU分解求本征矢量的逆
    gsl_matrix *pp = gsl_matrix_alloc(n, n);
    gsl_matrix *inverse = gsl_matrix_alloc(n, n);
    gsl_permutation *permutation = gsl_permutation_alloc(n);
    int s;

    gsl_matrix_memcpy(pp, p);

    gsl_linalg_LU_decomp(pp, permutation, &s);
    gsl_linalg_LU_invert(pp, permutation, inverse);
#ifdef DEBUG_s_root
    matrix_output(inverse, n, "本征矢量的逆:\n");
#endif
    gsl_matrix_free(pp);
    gsl_permutation_free(permutation);

    // 两个矩阵相乘，测试逆阵计算是否正确
    //gsl_matrix *c = gsl_matrix_calloc(n, n);
    //gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, p, inverse, 1.0, c);
    //matrix_output(c, n, "test:\n");
    //gsl_matrix_free(c);
    
    // 求得逆阵的平方根
    gsl_matrix *c = gsl_matrix_calloc(n, n);
    gsl_matrix *d = gsl_matrix_calloc(n, n);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, p, s_root, 1.0, c);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, c, inverse, 1.0, d);
#ifdef DEBUG_s_root
    matrix_output(d, n, "逆阵的平方根:\n");
#endif

    /*
    // 求本征矢量的逆
    //------------------ 不正确 ----------------------
    gsl_vector *v = gsl_vector_alloc(n);
    gsl_vector_set_all(v, 1.0);
    gsl_blas_dtrsv(CblasUpper, CblasNoTrans, CblasUnit, p, v);
    matrix_output(p, n, "本征矢量的逆:\n");

    */

    //printf("the inverse of A:\n");
    //gsl_matrix_fprintf(stdout, p, "%g");
    /*
     * ---------------------------------
     * 不是求本征值
     *----------------------------------
     *
    // 进行相似变换，求出本征值及变换矩阵
    gsl_linalg_balance_matrix(b, dialg_S);
    
    printf("对角化后:\n");
    gsl_vector_fprintf(stdout, dialg_S, "%g");
    printf("A :\n");
    gsl_matrix_fprintf(stdout, b, "%g");

    */

    gsl_matrix_free(b);
    gsl_matrix_free(p);
    //gsl_matrix_free(d);
    return d;
}

void matrix_output(gsl_matrix *m, int n, char *msg)
{
    int i, j;

    printf("%s",msg);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            printf("%20.11g", gsl_matrix_get(m, i, j));
        printf("\n");
    }
}

void vector_output(gsl_vector *v, int n, char *msg)
{
    int i;

    printf("%s",msg);
    for (i = 0; i < n; i++)
        printf("%20.11g", gsl_vector_get(v, i));
    printf("\n");
}
