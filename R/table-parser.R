
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Parse an SQLite \code{CREATE TABLE} statement into a nested list.
#' 
#' @param sql Character string containing an SQLite-compatible 
#'        \code{CREATE TABLE} statement.
#'        
#' @examples
#' \dontrun{
#' parse_sql("CREATE TABLE t1(x INTEGER PRIMARY KEY, y);")
#' }
#'         
#' @return a named list of information parsed from the \code{CREATE TABLE} 
#'         expression
#' \describe{
#'   \item{name}{name of database table}
#'   \item{schema}{the database scheme if available. otherwise NULL}
#'   \item{comment}{database comment if available}
#'   \item{temporary}{is this a temporary table?}
#'   \item{if_not_exists}{Create table if it does not exist}
#'   \item{without_rowid}{create table without rowid}
#'   \item{columns}{data.frame of column information
#'     \describe{
#'       \item{name}{column name}
#'       \item{type}{column type e.g. 'INTEGER', 'REAL', etc}
#'       \item{length}{length of value if given e.g \code{varchar(20)}}
#'       \item{constraint_name}{?}
#'       \item{comment}{column comment}
#'       \item{primary_key}{is this a primary key?}
#'       \item{auto_increment}{Should this auto increment?}
#'       \item{not_null}{is it defined as a 'NOT NULL' column}
#'       \item{unique}{should values be unique?}
#'       \item{order_pk}{ordering on the primary key. 'none', 'ascending' or 'descending'}
#'       \item{conflict_pk}{what to do when conflicts occur. 'none', 'rollback', 'abort', 'fail', 'ignore', 'replace'}
#'       \item{conflict_no_null}{What to do when 'no null' is violated. Same as \code{conflict_pk}}
#'       \item{conflict_unique}{What to do when uniqueness if violated. Same as above}
#'       \item{check_expr}{?}
#'       \item{default_expr}{Default value for this column}
#'       \item{collate_name}{?}
#'     }
#'   }
#'   \item{constraints}{data.frame of table contraints
#'     \describe{
#'       \item{name}{name of constraint}
#'       \item{type}{one of 'primary key', 'unique', 'check', 'foreign key'}
#'       \item{idx_cols}{?}
#'       \item{conflict_clause}{?}
#'       \item{check_expr}{?}
#'       \item{num_fk_cols}{?}
#'       \item{fk_cols}{?}
#'       \item{fk_table}{?}
#'       \item{fk_num_cols}{?}
#'       \item{fk_on_delete}{?}
#'       \item{fk_on_update}{?}
#'       \item{fk_match}{?}
#'     }
#'   }
#'   \item{type}{type of statement. One of 'unknown', 'table', 'rename table',
#'               'rename column', 'add column', 'drop column}
#'   \item{current_name}{the current name of the the table if needed}
#'   \item{new_name}{the new name of the table if needed}
#' }
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
parse_sql <- function(sql) {
  .Call(parse_, sql)
}