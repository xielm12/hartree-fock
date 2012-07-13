#include <gsl/gsl_vector.h>

#ifndef __INTEGRAL__BASIS__
#define __INTEGRAL__BASIS__
typedef struct gto {
    // gaussian = A * exp(-a * r^2)
    // A = A * (2a/pi)^(3/4)    仅对1S轨道
    int l, m, n;
    double alpha;
    double coeff;
    double norm;
}GTO;

typedef struct _b {
// the parameter of basis function
    int gauss_count;
    GTO* gaussian;   // 3 表示一个基函数由3个gaussian函数构成
    gsl_vector* xyz;
}BASIS;

typedef struct atom_INFORMATON_ {
// 定义一个原子信息, 包括核电荷数，基函数数目，元素符号，坐标，基函数
    int n;              // Atomic Number
    char symbol[3];     // Element Symbol
    gsl_vector* coordination;
}ATOM_INFO;

typedef struct FILE_INPUT_ {
// 存储解析后的整个输入文件
    int atomCount;          // atom count in current system
    int basisCount;    // basis function count
    BASIS** basisSet;   // save all of the basis set
    gsl_vector** gXYZ;   // save all of coordination
    ATOM_INFO** atomList;   // element information
}INPUT_INFO;

void* read_basis(const char * );
void basis_set_output(const BASIS*, int, char* );
void atom_output(const ATOM_INFO* atom, int n);

INPUT_INFO* parse_input(const char* file_name);
int readbasis(FILE * f, ATOM_INFO* atom_list, int atom_count);
double normalize_coeff(const GTO *);
#endif
