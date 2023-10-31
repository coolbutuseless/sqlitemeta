
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP parse_(SEXP sql_);

static const R_CallMethodDef CEntries[] = {
  
  {"parse_", (DL_FUNC) &parse_, 1},
  {NULL , NULL, 0}
};


void R_init_sqlitemeta(DllInfo *info) {
  R_registerRoutines(
    info,      // DllInfo
    NULL,      // .C
    CEntries,  // .Call
    NULL,      // Fortran
    NULL       // External
  );
  R_useDynamicSymbols(info, FALSE);
}



