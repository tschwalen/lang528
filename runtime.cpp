


/* 
Binary Operator valid type defitions

op          lhs         rhs
+           int/float   int/float
+           string      strin/int/float/bool/vec
/           int/float   int/float
-           int/float   int/float
*           int/float   int/float
%           int         int
==          [any]       [any]
!=          [any]       [any]
<=          int/float   int/float
>=          int/float   int/float
<           int/float   int/float
>           int/float   int/float
()          function    expr-list
[]          vec         int             (in future: lhs=indexable, rhs=key)
.           [any]       identifier
&           bool        bool
|           bool        bool

Unary operator valid type definitions
op          rhs
-           int/float
!           bool

*/