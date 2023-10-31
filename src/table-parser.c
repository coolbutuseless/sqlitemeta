


#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "sql3parse_table.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Helper for truning an sql3string to an R STRING
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP rstr(sql3string *str) {
  if (str == NULL) {
    return R_NilValue;
  }
  return mkString(sql3string_cstring(str));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Helper for truning an sql3string to an R CHAR
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP rchr(sql3string *str) {
  if (str == NULL) {
    return NA_STRING;
  }
  return mkChar(sql3string_cstring(str));
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Helper function list-to-data.frame
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void list_to_df(SEXP list_, unsigned int nrows) {
  
  unsigned int nprotect = 0;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Treat the VECSXP as a data.frame by setting the 'class' attribute
  // Also set some classes so that it appears as a tibble.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP class_ = PROTECT(allocVector(STRSXP, 3)); nprotect++;
  SET_STRING_ELT(class_, 0, mkChar("tbl_df"));
  SET_STRING_ELT(class_, 1, mkChar("tbl"));
  SET_STRING_ELT(class_, 2, mkChar("data.frame"));
  SET_CLASS(list_, class_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set the row.names on the list.
  // Use the shortcut as used in .set_row_names() in R
  // i.e. set rownames to c(NA_integer, -len) and it will
  // take care of the rest. This is equivalent to rownames(x) <- NULL
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP rownames = PROTECT(allocVector(INTSXP, 2));  nprotect++;
  SET_INTEGER_ELT(rownames, 0, NA_INTEGER);
  SET_INTEGER_ELT(rownames, 1, -nrows);
  setAttrib(list_, R_RowNamesSymbol, rownames);
  
  UNPROTECT(nprotect);
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// parse indexed column
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP parse_indexed_column(sql3tableconstraint *table_con) {

  unsigned int nprotect = 0;
  int N = sql3table_constraint_num_idxcolumns(table_con);
  
  if (N == 0) {
    return R_NilValue;
  }
  
  SEXP df_       = PROTECT(allocVector(VECSXP, 3)); nprotect++;
  SEXP df_names_ = PROTECT(allocVector(STRSXP, 3)); nprotect++;
  
  SET_STRING_ELT(df_names_,  0, mkChar("name"));  
  SET_STRING_ELT(df_names_,  1, mkChar("collate"));  
  SET_STRING_ELT(df_names_,  2, mkChar("order"));  
  
  setAttrib(df_, R_NamesSymbol, df_names_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP name_    = PROTECT(allocVector(STRSXP, N)); nprotect++;
  SEXP collate_ = PROTECT(allocVector(STRSXP, N)); nprotect++;
  SEXP order_   = PROTECT(allocVector(INTSXP, N)); nprotect++;
  
  SET_VECTOR_ELT(df_, 0, name_);
  SET_VECTOR_ELT(df_, 1, collate_);
  SET_VECTOR_ELT(df_, 2, order_);
  
  
  for (int i = 0; i < N; i++) {
    sql3idxcolumn *idx_col = sql3table_constraint_get_idxcolumn(table_con, i);
    SET_STRING_ELT(name_   , i, rchr(sql3idxcolumn_name   (idx_col)));
    SET_STRING_ELT(collate_, i, rchr(sql3idxcolumn_collate(idx_col)));
    INTEGER(order_)[i] = sql3idxcolumn_order(idx_col);
  }
  
  list_to_df(df_, N);
  
  UNPROTECT(nprotect);
  return df_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP parse_fk_column(sql3tableconstraint *table_con) {
  
  unsigned int nprotect = 0;
  int N = sql3table_constraint_num_fkcolumns(table_con);
  
  if (N == 0) {
    return R_NilValue;
  }
  
  SEXP vec_ = PROTECT(allocVector(STRSXP, N)); nprotect++;
  
  for (int i=0; i<N; i++) {
    SET_STRING_ELT(vec_, i, rchr(sql3table_constraint_get_fkcolumn(table_con, i)));
  }
  
  UNPROTECT(nprotect);
  return vec_;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP parse_fk_get_column(sql3foreignkey *fk) {
  
  unsigned int nprotect = 0;
  int N = sql3foreignkey_num_columns(fk);
  
  if (N == 0) {
    return R_NilValue;
  }
  
  SEXP vec_ = PROTECT(allocVector(STRSXP, N)); nprotect++;
  
  for (int i=0; i<N; i++) {
    SET_STRING_ELT(vec_, i, rchr(sql3foreignkey_get_column(fk, i)));
  }
  
  UNPROTECT(nprotect);
  return vec_;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Parse table constraints to a data.frame
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP parse_table_constraints(sql3table *table) {
  
  unsigned int nprotect = 0;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Constraints
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int N = sql3table_num_constraints(table);
  
  if (N == 0) {
    return R_NilValue;
  }
  
  SEXP df_       = PROTECT(allocVector(VECSXP, 14)); nprotect++;
  SEXP df_names_ = PROTECT(allocVector(STRSXP, 14)); nprotect++;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create names for constaints data.frame
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SET_STRING_ELT(df_names_,  0, mkChar("name"));  
  SET_STRING_ELT(df_names_,  1, mkChar("type"));  
  SET_STRING_ELT(df_names_,  2, mkChar("idx_cols")); // data.frame
  SET_STRING_ELT(df_names_,  3, mkChar("conflict_clause"));  
  SET_STRING_ELT(df_names_,  4, mkChar("check_expr"));  
  SET_STRING_ELT(df_names_,  5, mkChar("num_fk_cols"));  
  SET_STRING_ELT(df_names_,  6, mkChar("fk_cols")); // data.frame
  SET_STRING_ELT(df_names_,  7, mkChar("fk_table"));
  SET_STRING_ELT(df_names_,  8, mkChar("fk_num_cols"));
  SET_STRING_ELT(df_names_,  9, mkChar("fk_cols"));
  SET_STRING_ELT(df_names_, 10, mkChar("fk_on_delete"));
  SET_STRING_ELT(df_names_, 11, mkChar("fk_on_update"));
  SET_STRING_ELT(df_names_, 12, mkChar("fk_match"));
  SET_STRING_ELT(df_names_, 13, mkChar("fk_deferrable"));
  
  setAttrib(df_, R_NamesSymbol, df_names_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create SEXP vectors for each column in the data.frame
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP con_name_        = PROTECT(allocVector(STRSXP, N)); nprotect++; // 0
  SEXP con_type_        = PROTECT(allocVector(INTSXP, N)); nprotect++; // 1
  SEXP idx_cols_        = PROTECT(allocVector(VECSXP, N)); nprotect++; // 2
  SEXP con_conflict_    = PROTECT(allocVector(INTSXP, N)); nprotect++; // 3
  SEXP con_check_expr_  = PROTECT(allocVector(STRSXP, N)); nprotect++; // 4
  SEXP con_num_fk_cols_ = PROTECT(allocVector(INTSXP, N)); nprotect++; // 5
  SEXP fk_colnames_     = PROTECT(allocVector(VECSXP, N)); nprotect++; // 6
  
  // foreignkey clause
  SEXP fk_table_       = PROTECT(allocVector(STRSXP, N)); nprotect++; // 7
  SEXP fk_num_cols_    = PROTECT(allocVector(INTSXP, N)); nprotect++; // 8
  SEXP fk_cols_        = PROTECT(allocVector(VECSXP, N)); nprotect++; // 9
  SEXP fk_on_delete_   = PROTECT(allocVector(INTSXP, N)); nprotect++; // 10
  SEXP fk_on_update_   = PROTECT(allocVector(INTSXP, N)); nprotect++; // 11
  SEXP fk_match_       = PROTECT(allocVector(STRSXP, N)); nprotect++; // 12
  SEXP fk_deferrable_  = PROTECT(allocVector(INTSXP, N)); nprotect++; // 13
  
  SET_VECTOR_ELT(df_,  0, con_name_       );
  SET_VECTOR_ELT(df_,  1, con_type_       );
  SET_VECTOR_ELT(df_,  2, idx_cols_       );
  SET_VECTOR_ELT(df_,  3, con_conflict_   );
  SET_VECTOR_ELT(df_,  4, con_check_expr_ );
  SET_VECTOR_ELT(df_,  5, con_num_fk_cols_);
  SET_VECTOR_ELT(df_,  6, fk_colnames_    );
  SET_VECTOR_ELT(df_,  7, fk_table_       );
  SET_VECTOR_ELT(df_,  8, fk_num_cols_    );
  SET_VECTOR_ELT(df_,  9, fk_cols_        );
  SET_VECTOR_ELT(df_, 10, fk_on_delete_   );
  SET_VECTOR_ELT(df_, 11, fk_on_update_   );
  SET_VECTOR_ELT(df_, 12, fk_match_       );
  SET_VECTOR_ELT(df_, 13, fk_deferrable_  );
  
  
  for (int i = 0; i < N; i++) {
    sql3tableconstraint *table_con = sql3table_get_constraint(table, i);
    
    SET_STRING_ELT(con_name_, i, rchr(sql3table_constraint_name(table_con)));             // 0
    INTEGER(con_type_    )[i] = 1 + sql3table_constraint_type(table_con);                 // 2
    SET_VECTOR_ELT(idx_cols_, i, parse_indexed_column(table_con));                        // 3
    INTEGER(con_conflict_)[i] = sql3table_constraint_conflict_clause(table_con);          // 4
    SET_STRING_ELT(con_check_expr_, i, rchr(sql3table_constraint_check_expr(table_con))); // 5
    INTEGER(con_num_fk_cols_)[i] = sql3table_constraint_num_fkcolumns(table_con);         // 6
    SET_VECTOR_ELT(fk_colnames_, i, parse_fk_column(table_con));                          // 7
    
    sql3foreignkey *fk = sql3table_constraint_foreignkey_clause(table_con);
    if (fk != NULL) {
      SET_STRING_ELT(fk_table_, i, rchr(sql3foreignkey_table(fk)));       //  8
      INTEGER(fk_num_cols_)[i] = sql3foreignkey_num_columns(fk);          //  9
      SET_VECTOR_ELT(fk_cols_, i, parse_fk_get_column(fk));               // 10
      INTEGER(fk_on_delete_)[i] = 1 + sql3foreignkey_ondelete_action(fk); // 11
      INTEGER(fk_on_update_)[i] = 1 + sql3foreignkey_onupdate_action(fk); // 12
      SET_STRING_ELT(fk_match_, i, rchr(sql3foreignkey_match(fk)));       // 13
      INTEGER(fk_deferrable_)[i] = 1 + sql3foreignkey_deferrable(fk);     // 14
    } else {
      SET_STRING_ELT(fk_table_, i, NA_STRING); //  8
      INTEGER(fk_num_cols_)[i] = NA_INTEGER;   //  9
      SET_VECTOR_ELT(fk_cols_, i, R_NilValue); // 10
      INTEGER(fk_on_delete_)[i] = NA_INTEGER;  // 11
      INTEGER(fk_on_update_)[i] = NA_INTEGER;  // 12
      SET_STRING_ELT(fk_match_, i, NA_STRING); // 13
      INTEGER(fk_deferrable_)[i] = NA_INTEGER; // 14
    }
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set factors
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  {
    // SQL3FKACTION_NONE,
    // SQL3FKACTION_SETNULL,
    // SQL3FKACTION_SETDEFAULT,
    // SQL3FKACTION_CASCADE,
    // SQL3FKACTION_RESTRICT,
    // SQL3FKACTION_NOACTION
    SEXP levels_ = PROTECT(allocVector(STRSXP, 6)); 
    SET_STRING_ELT(levels_, 0, mkChar("none"));
    SET_STRING_ELT(levels_, 1, mkChar("set null"));
    SET_STRING_ELT(levels_, 2, mkChar("set default"));
    SET_STRING_ELT(levels_, 3, mkChar("cascade"));
    SET_STRING_ELT(levels_, 4, mkChar("restrict"));
    SET_STRING_ELT(levels_, 5, mkChar("no action"));
    
    setAttrib(fk_on_delete_, R_ClassSymbol, mkString("factor"));
    setAttrib(fk_on_delete_, R_LevelsSymbol, levels_);
    
    setAttrib(fk_on_update_, R_ClassSymbol, mkString("factor"));
    setAttrib(fk_on_update_, R_LevelsSymbol, levels_);
    
    UNPROTECT(1);
  }
  
  {
    // SQL3DEFTYPE_NONE,
    // SQL3DEFTYPE_DEFERRABLE,
    // SQL3DEFTYPE_DEFERRABLE_INITIALLY_DEFERRED,
    // SQL3DEFTYPE_DEFERRABLE_INITIALLY_IMMEDIATE,
    // SQL3DEFTYPE_NOTDEFERRABLE,
    // SQL3DEFTYPE_NOTDEFERRABLE_INITIALLY_DEFERRED,
    // SQL3DEFTYPE_NOTDEFERRABLE_INITIALLY_IMMEDIATE
    SEXP levels_ = PROTECT(allocVector(STRSXP, 7)); 
    SET_STRING_ELT(levels_, 0, mkChar("none"));
    SET_STRING_ELT(levels_, 1, mkChar("deferrable"));
    SET_STRING_ELT(levels_, 2, mkChar("deferrable initially deferred"));
    SET_STRING_ELT(levels_, 3, mkChar("deferrable initially immediate"));
    SET_STRING_ELT(levels_, 4, mkChar("not deferrable"));
    SET_STRING_ELT(levels_, 5, mkChar("not deferrable initially deferred"));
    SET_STRING_ELT(levels_, 6, mkChar("not deferrable initially immediate"));
    
    setAttrib(fk_deferrable_, R_ClassSymbol, mkString("factor"));
    setAttrib(fk_deferrable_, R_LevelsSymbol, levels_);
    
    UNPROTECT(1);
  }
  
  {
    // SQL3TABLECONSTRAINT_PRIMARYKEY,
    // SQL3TABLECONSTRAINT_UNIQUE,
    // SQL3TABLECONSTRAINT_CHECK,
    // SQL3TABLECONSTRAINT_FOREIGNKEY
    SEXP levels_ = PROTECT(allocVector(STRSXP, 4)); 
    SET_STRING_ELT(levels_, 0, mkChar("primary key"));
    SET_STRING_ELT(levels_, 1, mkChar("unique"));
    SET_STRING_ELT(levels_, 2, mkChar("check"));
    SET_STRING_ELT(levels_, 3, mkChar("foreign key"));
    
    setAttrib(con_type_, R_ClassSymbol, mkString("factor"));
    setAttrib(con_type_, R_LevelsSymbol, levels_);
    
    UNPROTECT(1);
  }
  
  list_to_df(df_, N);
  
  UNPROTECT(nprotect);
  return df_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Parse database column information into a data.frame
//
// @return data.frame
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP parse_column_info(sql3table *table) {
  
  unsigned int nprotect = 0;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Column Info
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int N = sql3table_num_columns(table);
  
  if (N == 0) {
    return R_NilValue;
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create columns data.frame and name it
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP df_       = PROTECT(allocVector(VECSXP, 16)); nprotect++;
  SEXP df_names_ = PROTECT(allocVector(STRSXP, 16)); nprotect++;
  
  SET_STRING_ELT(df_names_,  0, mkChar("name"));  
  SET_STRING_ELT(df_names_,  1, mkChar("type"));  
  SET_STRING_ELT(df_names_,  2, mkChar("length"));  
  SET_STRING_ELT(df_names_,  3, mkChar("constraint_name"));  
  SET_STRING_ELT(df_names_,  4, mkChar("comment"));
  
  SET_STRING_ELT(df_names_,  5, mkChar("primary_key"));
  SET_STRING_ELT(df_names_,  6, mkChar("auto_increment"));
  SET_STRING_ELT(df_names_,  7, mkChar("not_null"));
  SET_STRING_ELT(df_names_,  8, mkChar("unique"));
  
  SET_STRING_ELT(df_names_,  9, mkChar("order_pk"));
  SET_STRING_ELT(df_names_, 10, mkChar("conflict_pk"));
  SET_STRING_ELT(df_names_, 11, mkChar("conflict_no_null"));
  SET_STRING_ELT(df_names_, 12, mkChar("conflict_unique"));
  
  SET_STRING_ELT(df_names_, 13, mkChar("check_expr"));
  SET_STRING_ELT(df_names_, 14, mkChar("default_expr"));
  SET_STRING_ELT(df_names_, 15, mkChar("collate_name"));
  
  setAttrib(df_, R_NamesSymbol, df_names_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create SEXP vectors for each column and place in data.frame
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP col_names_  = PROTECT(allocVector(STRSXP, N)); nprotect++;
  SEXP col_types_  = PROTECT(allocVector(STRSXP, N)); nprotect++;
  SEXP col_lens_   = PROTECT(allocVector(STRSXP, N)); nprotect++;
  SEXP col_cnames_ = PROTECT(allocVector(STRSXP, N)); nprotect++;
  SEXP col_comm_   = PROTECT(allocVector(STRSXP, N)); nprotect++;
  
  SEXP col_primkey_ = PROTECT(allocVector(LGLSXP, N)); nprotect++;
  SEXP col_autoinc_ = PROTECT(allocVector(LGLSXP, N)); nprotect++;
  SEXP col_notnull_ = PROTECT(allocVector(LGLSXP, N)); nprotect++;
  SEXP col_unique_  = PROTECT(allocVector(LGLSXP, N)); nprotect++;
  
  SEXP col_order_         = PROTECT(allocVector(INTSXP, N)); nprotect++;
  SEXP col_conf_pk_       = PROTECT(allocVector(INTSXP, N)); nprotect++;
  SEXP col_conf_not_null_ = PROTECT(allocVector(INTSXP, N)); nprotect++;
  SEXP col_conf_unique_   = PROTECT(allocVector(INTSXP, N)); nprotect++;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set factors
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  {
    // Order 
    SEXP order_levels_ = PROTECT(allocVector(STRSXP, 3)); 
    SET_STRING_ELT(order_levels_, 0, mkChar("none"));
    SET_STRING_ELT(order_levels_, 1, mkChar("ascending"));
    SET_STRING_ELT(order_levels_, 2, mkChar("descending"));
    setAttrib(col_order_, R_ClassSymbol, mkString("factor"));
    setAttrib(col_order_, R_LevelsSymbol, order_levels_);
    UNPROTECT(1);
    
    
    // Conflict 
    // SQL3CONFLICT_NONE,
    // SQL3CONFLICT_ROLLBACK,
    // SQL3CONFLICT_ABORT,
    // SQL3CONFLICT_FAIL,
    // SQL3CONFLICT_IGNORE,
    // SQL3CONFLICT_REPLACE
    SEXP conflict_levels_ = PROTECT(allocVector(STRSXP, 6)); 
    SET_STRING_ELT(conflict_levels_, 0, mkChar("none"));
    SET_STRING_ELT(conflict_levels_, 1, mkChar("rollback"));
    SET_STRING_ELT(conflict_levels_, 2, mkChar("abort"));
    SET_STRING_ELT(conflict_levels_, 3, mkChar("fail"));
    SET_STRING_ELT(conflict_levels_, 4, mkChar("ignore"));
    SET_STRING_ELT(conflict_levels_, 5, mkChar("replace"));
    
    setAttrib(col_conf_pk_      , R_ClassSymbol, mkString("factor"));
    setAttrib(col_conf_not_null_, R_ClassSymbol, mkString("factor"));
    setAttrib(col_conf_unique_  , R_ClassSymbol, mkString("factor"));
    
    setAttrib(col_conf_pk_      , R_LevelsSymbol, conflict_levels_);
    setAttrib(col_conf_not_null_, R_LevelsSymbol, conflict_levels_);
    setAttrib(col_conf_unique_  , R_LevelsSymbol, conflict_levels_);
    UNPROTECT(1);
  }
  
  SEXP col_check_expr_   = PROTECT(allocVector(STRSXP, N)); nprotect++;
  SEXP col_default_expr_ = PROTECT(allocVector(STRSXP, N)); nprotect++;
  SEXP col_collate_name_ = PROTECT(allocVector(STRSXP, N)); nprotect++;
  
  SET_VECTOR_ELT(df_,  0, col_names_);
  SET_VECTOR_ELT(df_,  1, col_types_);
  SET_VECTOR_ELT(df_,  2, col_lens_);
  SET_VECTOR_ELT(df_,  3, col_cnames_);
  SET_VECTOR_ELT(df_,  4, col_comm_);
  
  SET_VECTOR_ELT(df_,  5, col_primkey_);
  SET_VECTOR_ELT(df_,  6, col_autoinc_);
  SET_VECTOR_ELT(df_,  7, col_notnull_);
  SET_VECTOR_ELT(df_,  8, col_unique_);
  
  SET_VECTOR_ELT(df_,  9, col_order_);
  SET_VECTOR_ELT(df_, 10, col_conf_pk_);
  SET_VECTOR_ELT(df_, 11, col_conf_not_null_);
  SET_VECTOR_ELT(df_, 12, col_conf_unique_);
  
  SET_VECTOR_ELT(df_, 13, col_check_expr_);
  SET_VECTOR_ELT(df_, 14, col_default_expr_);
  SET_VECTOR_ELT(df_, 15, col_collate_name_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Populate the data.frame columns 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  for (int col_idx = 0; col_idx < N; col_idx++) {
    
    sql3column *col = sql3table_get_column(table, col_idx);
    
    SET_STRING_ELT(col_names_ , col_idx, rchr(sql3column_name(col)));
    SET_STRING_ELT(col_types_ , col_idx, rchr(sql3column_type(col)));
    SET_STRING_ELT(col_lens_  , col_idx, rchr(sql3column_length(col)));
    SET_STRING_ELT(col_cnames_, col_idx, rchr(sql3column_constraint_name(col)));
    SET_STRING_ELT(col_comm_  , col_idx, rchr(sql3column_comment(col)));
    
    LOGICAL(col_primkey_)[col_idx] = sql3column_is_primarykey(col);
    LOGICAL(col_autoinc_)[col_idx] = sql3column_is_autoincrement(col);
    LOGICAL(col_notnull_)[col_idx] = sql3column_is_notnull(col);
    LOGICAL(col_unique_ )[col_idx] = sql3column_is_unique(col);
    
    INTEGER(col_order_        )[col_idx] = 1 + sql3column_pk_order(col);
    INTEGER(col_conf_pk_      )[col_idx] = 1 + sql3column_pk_conflictclause(col);
    INTEGER(col_conf_not_null_)[col_idx] = 1 + sql3column_notnull_conflictclause(col);
    INTEGER(col_conf_unique_  )[col_idx] = 1 + sql3column_unique_conflictclause(col);
    
    SET_STRING_ELT(col_check_expr_  , col_idx, rchr(sql3column_check_expr(col)));
    SET_STRING_ELT(col_default_expr_, col_idx, rchr(sql3column_default_expr(col)));
    SET_STRING_ELT(col_collate_name_, col_idx, rchr(sql3column_collate_name(col)));
  }
  
  list_to_df(df_, N);
  
  UNPROTECT(nprotect);
  return df_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Parse CREATE TABLE
//
// @param sql_ an sqlite3 "CREATE TABLE" statement
// @return named list of information about the table
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP parse_(SEXP sql_) {
  
  unsigned int nprotect = 0;
  sql3error_code err;
  sql3table *table = sql3parse_table(CHAR(asChar(sql_)), 0, &err);
  
  if (table == NULL) {
    error("Couldn't parse CREATE TABLE from given sql");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Assemble 'table_info' list
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  static char *types[] = {
    "unknown",
    "table",
    "rename table",
    "rename column",
    "add column",
    "drop column"
  };
  
  SEXP table_info_  = PROTECT(allocVector(VECSXP, 11)); nprotect++;
  SEXP table_names_ = PROTECT(allocVector(STRSXP, 11)); nprotect++;
  SET_STRING_ELT(table_names_,  0, mkChar("name"));  
  SET_STRING_ELT(table_names_,  1, mkChar("schema"));
  SET_STRING_ELT(table_names_,  2, mkChar("comment"));
  SET_STRING_ELT(table_names_,  3, mkChar("temporary"));
  SET_STRING_ELT(table_names_,  4, mkChar("if_not_exists"));
  SET_STRING_ELT(table_names_,  5, mkChar("without_rowid"));
  SET_STRING_ELT(table_names_,  6, mkChar("columns"));
  SET_STRING_ELT(table_names_,  7, mkChar("constraints"));
  SET_STRING_ELT(table_names_,  8, mkChar("type"));
  SET_STRING_ELT(table_names_,  9, mkChar("current_name"));
  SET_STRING_ELT(table_names_, 10, mkChar("new_name"));
  setAttrib(table_info_, R_NamesSymbol, table_names_);
  
  SET_VECTOR_ELT(table_info_,  0, rstr(sql3table_name  (table)));
  SET_VECTOR_ELT(table_info_,  1, rstr(sql3table_schema(table)));
  SET_VECTOR_ELT(table_info_,  2, rstr(sql3table_comment(table)));
  SET_VECTOR_ELT(table_info_,  3, ScalarLogical(sql3table_is_temporary(table)));
  SET_VECTOR_ELT(table_info_,  4, ScalarLogical(sql3table_is_ifnotexists(table)));
  SET_VECTOR_ELT(table_info_,  5, ScalarLogical(sql3table_is_withoutrowid(table)));
  SET_VECTOR_ELT(table_info_,  6, parse_column_info(table));
  SET_VECTOR_ELT(table_info_,  7, parse_table_constraints(table));
  SET_VECTOR_ELT(table_info_,  8, mkString(types[sql3table_type(table)]));
  SET_VECTOR_ELT(table_info_,  9, rstr(sql3table_current_name(table)));
  SET_VECTOR_ELT(table_info_, 10, rstr(sql3table_new_name(table)));
  
  
  sql3table_free(table);
  UNPROTECT(nprotect);
  return table_info_;
}



