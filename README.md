
<!-- README.md is generated from README.Rmd. Please edit that file -->

# sqlitemeta

<!-- badges: start -->

![](https://img.shields.io/badge/cool-useless-green.svg)
<!-- badges: end -->

`{sqlitemeta}` is a tool for parsing SQLite `CREATE TABLE` statements.

[SQLite](https://sqlite.org) stores and updates information about the
columns it contains in a stored SQL statement as part of the database.

This stored statement is an SQL `CREATE TABLE` operation describing how
you could the create the table in question.

So if you want to learn about the structure of an existing database,
parsing this `CREATE TABLE` statement is a key part of understanding how
it was constructed.

This package is wrapper around the C libary
[sqlite-createtable-parser](https://github.com/marcobambini/sqlite-createtable-parser).

## What’s in the box

- `parse_sql()` will parse a SQLite `CREATE TABLE` statement (provided
  as a single character string) and return a named list of information.

## Installation

You can install from
[GitHub](https://github.com/coolbutuseless/sqlitemeta) with:

``` r
# install.package('remotes')
remotes::install_github('coolbutuseless/sqlitemeta')
```

## Example 1 - Parsing a provided `CREATE TABLE` statement

``` r
library(sqlitemeta)

sql <- r"(
CREATE TABLE contact_groups(                
   contact_id INTEGER PRIMARY KEY,      
   group_id INTEGER DEFAULT 999,
   details VARCHAR(20),
   PRIMARY KEY (contact_id, group_id),      
   FOREIGN KEY (contact_id)                 
      REFERENCES contacts (contact_id)      
         ON DELETE CASCADE                  
         ON UPDATE NO ACTION,               
   FOREIGN KEY (group_id)                   
      REFERENCES groups (group_id)          
         ON DELETE CASCADE                  
         ON UPDATE NO ACTION                
) STRICT;
)"

parse_sql(sql)
#> $name
#> [1] "contact_groups"
#> 
#> $schema
#> NULL
#> 
#> $comment
#> NULL
#> 
#> $temporary
#> [1] FALSE
#> 
#> $if_not_exists
#> [1] FALSE
#> 
#> $without_rowid
#> [1] FALSE
#> 
#> $columns
#>         name    type length constraint_name comment primary_key auto_increment
#> 1 contact_id INTEGER   <NA>            <NA>    <NA>        TRUE          FALSE
#> 2   group_id INTEGER   <NA>            <NA>    <NA>       FALSE          FALSE
#> 3    details VARCHAR     20            <NA>    <NA>       FALSE          FALSE
#>   not_null unique order_pk conflict_pk conflict_no_null conflict_unique
#> 1    FALSE  FALSE     none        none             none            none
#> 2    FALSE  FALSE     none        none             none            none
#> 3    FALSE  FALSE     none        none             none            none
#>   check_expr default_expr collate_name
#> 1       <NA>         <NA>         <NA>
#> 2       <NA>          999         <NA>
#> 3       <NA>         <NA>         <NA>
#> 
#> $constraints
#>   name        type                           idx_cols conflict_clause
#> 1 <NA> primary key contact_id, group_id, NA, NA, 0, 0               0
#> 2 <NA> foreign key                               NULL               0
#> 3 <NA> foreign key                               NULL               0
#>   check_expr num_fk_cols    fk_cols fk_table fk_num_cols    fk_cols
#> 1       <NA>           0       NULL     <NA>          NA       NULL
#> 2       <NA>           1 contact_id contacts           1 contact_id
#> 3       <NA>           1   group_id   groups           1   group_id
#>   fk_on_delete fk_on_update fk_match fk_deferrable
#> 1         <NA>         <NA>     <NA>          <NA>
#> 2      cascade    no action     <NA>          none
#> 3      cascade    no action     <NA>          none
#> 
#> $type
#> [1] "table"
#> 
#> $current_name
#> NULL
#> 
#> $new_name
#> NULL
```

## Example 2 - Extracting `CREATE TABLE` statement from existing SQLite database

``` r
library(sqlitemeta)
library(RSQLite)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Create an in-memory SQLite database and add 'mtcars' data as a table
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
con <- dbConnect(RSQLite::SQLite(), ":memory:")
dbWriteTable(con, "mtcars", mtcars, overwrite = TRUE)
dbListTables(con)
#> [1] "mtcars"

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Query Sqlite for the 'CREATE TABLE' statement used to 
# create the 'mtcars' table in the database.
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
qry <- "SELECT sql FROM sqlite_master WHERE name = 'mtcars';"
res <- dbSendQuery(con, qry)
res <- dbFetch(res)
create_table_sql <- res$sql[[1]] 

cat(create_table_sql, sep = "\n")
#> CREATE TABLE `mtcars` (
#>   `mpg` REAL,
#>   `cyl` REAL,
#>   `disp` REAL,
#>   `hp` REAL,
#>   `drat` REAL,
#>   `wt` REAL,
#>   `qsec` REAL,
#>   `vs` REAL,
#>   `am` REAL,
#>   `gear` REAL,
#>   `carb` REAL
#> )

parse_sql(create_table_sql)
#> $name
#> [1] "mtcars"
#> 
#> $schema
#> NULL
#> 
#> $comment
#> NULL
#> 
#> $temporary
#> [1] FALSE
#> 
#> $if_not_exists
#> [1] FALSE
#> 
#> $without_rowid
#> [1] FALSE
#> 
#> $columns
#>    name type length constraint_name comment primary_key auto_increment not_null
#> 1   mpg REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#> 2   cyl REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#> 3  disp REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#> 4    hp REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#> 5  drat REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#> 6    wt REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#> 7  qsec REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#> 8    vs REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#> 9    am REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#> 10 gear REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#> 11 carb REAL   <NA>            <NA>    <NA>       FALSE          FALSE    FALSE
#>    unique order_pk conflict_pk conflict_no_null conflict_unique check_expr
#> 1   FALSE     none        none             none            none       <NA>
#> 2   FALSE     none        none             none            none       <NA>
#> 3   FALSE     none        none             none            none       <NA>
#> 4   FALSE     none        none             none            none       <NA>
#> 5   FALSE     none        none             none            none       <NA>
#> 6   FALSE     none        none             none            none       <NA>
#> 7   FALSE     none        none             none            none       <NA>
#> 8   FALSE     none        none             none            none       <NA>
#> 9   FALSE     none        none             none            none       <NA>
#> 10  FALSE     none        none             none            none       <NA>
#> 11  FALSE     none        none             none            none       <NA>
#>    default_expr collate_name
#> 1          <NA>         <NA>
#> 2          <NA>         <NA>
#> 3          <NA>         <NA>
#> 4          <NA>         <NA>
#> 5          <NA>         <NA>
#> 6          <NA>         <NA>
#> 7          <NA>         <NA>
#> 8          <NA>         <NA>
#> 9          <NA>         <NA>
#> 10         <NA>         <NA>
#> 11         <NA>         <NA>
#> 
#> $constraints
#> NULL
#> 
#> $type
#> [1] "table"
#> 
#> $current_name
#> NULL
#> 
#> $new_name
#> NULL
```

## Related Software

- [RSQLite](https://cran.dev/RSQLite)

## Acknowledgements

- R Core for developing and maintaining the language.
- CRAN maintainers, for patiently shepherding packages onto CRAN and
  maintaining the repository
