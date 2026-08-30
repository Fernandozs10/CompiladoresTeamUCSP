# ChocoPy v2.2: Language Manual and Reference

### Designed by Rohan Padhye and Koushik Sen; v2 changes by Paul Hilfnger

### University of California, Berkeley

### November 23, 2019

## Contents

1 Introduction 3

2 A tour of ChocoPy 3

2.1 The top level.............................................4
2.2 Functions.............................................. .4
2.3 Classes. ................ .. .. .. .. ....................... .5
2.4 Type hierarchy. ...........................................6
2.5 Values................................................ .7
2.5.1 Integers............ .. .. .. .. .. ..................... .7
2.5.2 Booleans. ......... .. .. .. .. .. .. .....................7
2.5.3 Strings .............................................7
2.5.4 Lists. ............. .. .. ...... ..................... .7
2.5.5 Objects of user-dened classes.............................. .7
2.5.6 None. ... .......... .. .. .. .. .. ..................... .8
2.5.7 The empty list ([])...... .. .. .. .. .. ..................... .8
2.6 Expressions............................................. .8
2.6.1 Literals and identiers. .. .. .. .. .. .........................8
2.6.2 List expressions. ...... .. .. .. .. .. .. .....................8
2.6.3 Arithmetic expressions................................... .8
2.6.4 Logical expressions.................................... ..8
2.6.5 Relational expressions. .. .. .. .. .. .. .. .................... .9
2.6.6 Conditional expressions.. .. .. .. .. .. .. .................... .9
2.6.7 Concatenation expressions................................ ..9
2.6.8 Access expressions......................................9
2.6.9 Call expressions ....................................... .9
2.7 Type annotations .......................................... .9
2.8 Statements. ............. ...... .. ........................10
2.8.1 Expression statements. .. .. .. .. .. .. .. .....................10
2.8.2 Compound statements: conditionals and loops......................10
2.8.3 Assignment statements.................................. ..11
2.8.4 Pass statement........ .. .. .. .. .. ......................11
2.8.5 Return statement ...... .. .. .. .. .. .. .....................11
2.8.6 Predened classes and functions..............................11

3 Lexical structure 12

3.1 Line structure ............ .. .. .. .. .........................12
3.1.1 Physical lines........ .. .. .. .. .........................12
3.1.2 Logical lines .......... .. .. .. .. .. ......................12
3.1.3 Comments.. .........................................12
3.1.4 Blank lines. .........................................12
3.1.5 Indentation. .........................................12
3.1.6 Whitespace between tokens. .. .. .. .. .. ......................13
3.2 Identiers...............................................13
3.3 Keywords...............................................13
3.4 Literals................ .. .. .. .. ........................13
3.4.1 String literals. ........................................13
3.4.2 Integer literals....... .. .. .. .. .. .. .....................14
3.5 Operators and delimiters...................................... .14
4 Syntax 14

4.1 Precedence and Associativity.... .. .. .. .. ........................14
5 Type rules 16

5.1 Type environments ..........................................16
5.2 Type checking rules.........................................17
6 Operational semantics 23

6.1 Evaluation context..........................................24
6.2 Syntax for values. ......... .. .. .. .. .........................25
6.2.1 Class instances........ .. .. .. .. .. ......................25
6.2.2 List objects..........................................26
6.2.3 None.. ................. .. .. .. .. .. .. ...............26
6.2.4 Functions...........................................26
6.3 Syntax for class denitions..................................... .26
6.4 Operational rules.......... .. .. .. .. .........................26
#### 7 Acknowledgements 38

#### A Known incompatibilities with Python 38

## 1 Introduction

This manual describes the ChocoPy language, which is a statically typed dialect of Python 3.6. ChocoPy is intended to be used in a classroom setting. It has been designed to be small enough for students to implement a full ChocoPy compiler over one semester. ChocoPy has been designed to be a subset of Python. Almost every valid ChocoPy program is also a valid Python 3.6 program. An execution of a ChocoPy program that does not result in error usually has the same observable semantics as the execution of that program in Python 3.6. Appendix A lists the small number of exceptions to this rule. A ChocoPy program is contained in a single source le. At the top level, a ChocoPy program consists of a sequence of variable denitions, function denitions, and class denitions followed by a sequence of statements. A class consists of a sequence of attribute denitions and method denitions. A class creates a user-dened type. Function denitions can be nested inside other methods and functions. All class names and functions dened at the top level are globally visible. Classes, functions, and methods cannot be redened. Program statements can contain expressions, assignments, and control-ow statements such as conditionals and loops. Evaluation of an expression results in a value that can be an integer, a boolean, a string, an object of user-dened class, a list, or the special value None. ChocoPy does not support dictionaries, rst-class functions, and reective introspection. All expressions are statically typed. Variables (global and local) and class attributes are statically typed, and have only one type throughout their lifetime. Both variables and attributes are explicitly typed using annotations. In function and method denitions, type annotations are used to explicitly specify return type and types of formal parameters. For readers familiar with the Python language, Figure 1 contains a sample ChocoPy program illustrating top-level functions, statements, global variables, local variables, and type annotations. The type annotations are valid syntaxes in Python 3.6, though the Python interpreter simply ignores these annotations and leaves them as hints for other tools. In contrast, ChocoPy enforces static type checking at compile time. In

Figure 1, the function is zero is dened at the top level. Its formal parameters items and idx are explicitly

typed as a list of integers and an integer, respectively. The return type of the function is zero is bool. The function denes a local variable, val, whose type is int. At the top level, the program denes a global variable mylist, whose type is a list of integers. Function is zero is invoked in a top level statement and its result is output using the predened print function. Similarly, Figure 2 contains a ChocoPy program that denes two classes: animal and cow. The class cow inherits from animal, which in turn inherits from the predened root class object. The Boolean attribute makes noise is dened in animal and is therefore inherited by class cow. The class cow overrides the method sound. The constructor for class cow is invoked at line 19. Section 2 provides a detailed but informal overview of the various language constructs in ChocoPy. Sections 3{6 provide formal descriptions of the lexical structure, grammatical syntax, typing rules, and operation semantics of ChocoPy.

## 2 A tour of ChocoPy

Notation In the rest of this section, we use:

*f*expr*g* to denote an expression in the program.

*f*id*g* to denote an identier such as the name of a variable or function.

*f*stmts*g* to denote a list of program statements, separated by newlines.

*f*declarations*g* to denote a list of (possibly interleaved) declarations of functions, variables, attributes, and/or classes, where applicable.

*f*type*g* to denote a static type annotation.

*f*literal*g* to denote a constant literal such as an integer literal, a string literal, or the keywords True, False or None.

||defis_zero (items :||[int], idx :int) ->bool:|
|---|---|---|---|
||val :int=||0#Typeisexplicitlydeclared|
|val|= returnval|items [idx] ==|0|
|mylist :|[int]|=|None|
|mylist|= [1, print(is_zero (mylist,|0, 1]|1))#Prints’True’|

1 2 3 4 5 6 7 8

Figure 1: ChocoPy program illustrating functions, variables, and static typing.

### 2.1 The top level

A ChocoPy program consists of zero or more denitions followed by zero or more statements, referred to as top-level denitions and statements respectively. Top-level denitions include global variable denitions, function denitions, and class denitions. These denitions create new mappings in a scope called the *global scope*. Global variables are dened using the syntax *f*id*g*: *f*type*g* = *f*literal*g*, where the identier species the variable name, the type annotation species the static type of the variable, and the constant literal zspecies the initial value of the variable upon program execution. The names of global variables, global functions, and classes must be distinct. Top-level statements execute in the global scope; that is, expressions in top-level statements may reference entities dened in the global scope using identiers. A ChocoPy program’s execution begins with the rst top-level statement and ends when the last top-level statement is executed completely. Function and class denitions are described in Section 2.2 and Section 2.3 respectively. Program statements are described in Section 2.8.

### 2.2 Functions

In ChocoPy, a function denition can appear at the top level of a program, or it could be nested inside other functions or methods. Functions cannot be redened in the same scope. However, a function denition in the current scope can shadow a function dened in a surrounding scope of the function. A function denition has the following form:

def {id}({id}: {type},..., {id}: {type}) {return type}: {declarations} {stmts}

where {return type} is either empty or has the form -> {type}. The rst line denes the function’s name, a comma-separated list of zero or more formal parameters in parentheses, and the function’s return type after the -> symbol. When the return type is empty, the function may only return the value None. Every formal parameter has a name and a static type annotation. The body of a function contains a sequence of zero or more declarations followed by a sequence of one or more program statements. A function denition creates a new scope. Declarations in a function body include local variable denitions, global and nonlocal variable declarations, and denitions of nested functions. *f*id*g*: *f*type*g* = *f*literal*g*. Such a denition declares a local variable with the name id, explicitly associates it with a static type, and species an initial value using a literal. The global *f*id*g* statement is used to bind a name to a global variable. Similarly, nonlocal *f*id*g* statement is used within a nested function to bind a name to a variable dened in a surrounding scope that is not the global scope|specically to the closest surrounding scope declaring that variable. It is illegal for a global declaration to occur at the top level. Similarly, it is illegal for a nonlocal declaration to occur outside a nested function, or to refer to a global variable. If a variable is not explicitly declared in a function, but is bound to some entity|variable, function, or class|in any surrounding scope, then its binding is implicitly inherited from the surrounding scope as a read-only variable|such a variable cannot be assigned to in any of the function’s statements.

1 classanimal (object) : 2 makes_noise :bool= False 3 4 defmake_noise ( self :"animal")->object: 5 if( self. makes_noise ) : 6 print( self. sound () ) 7 8 defsound ( self :"animal")->str: 9 return"???" 10 11 classcow ( animal ) : 12 def__init__ ( self :"cow") : 13 self. makes_noise = True 14 15 defsound ( self :"cow")->str: 16 return"moo" 17 18 c : animal = None 19 c = cow () 20 c. make_noise ()#Prints"moo"

Figure 2: ChocoPy program illustrating classes, attributes, methods, and inheritance.

### 2.3 Classes

In ChocoPy, a class denition can appear at the top level of a program. Classes cannot be redened. Class names can never be shadowed; that is, a program may not dene any variable or function with the same name as a class name. A class denition has the following form:

class {id}({id}): {declarations}

The rst line species the name of the class followed by the name of its *superclass* in parentheses. The class name must not be be bound to any other entity|class, function, or variable|in the program. The superclass must refer to a class that has been previously dened in the program, or be the predened class object. The superclass may not be one of int, str, or bool. The class body consists of a sequence of attribute denitions and method denitions. In ChocoPy, attributes and methods are associated with object instances of a class, and not with the classes themselves; that is, ChocoPy does not support the notion of *static* class members that some other languages support. Similar to variable denitions, an attribute denition has the form *f*id*g*: *f*type*g* = *f*literal*g*. A method denition has the same syntax as a function denitions (ref. Section 2.2), with two important restrictions:

(1) a method denition must have at least one formal parameter, and (2) the rst formal parameter must have the dening class as type. A class denes attributes and methods. A class inherits attributes and methods of its superclass. Attributes, whether dened in the current class or inherited from the superclass, cannot be redened. Methods cannot be redened in the same class. Inherited methods can be redened as long as the return type and the types of all formal parameters except the rst parameter are exactly the same. Any reference to an attribute or method must be prexed with an expression and the dot operator (ref. Section 2.6). If a class C is dened to have a superclass P, then class C is a subclass of P. If C is a subclass of P, then P must either be a user-dened class that has been dened before C in the program, or be the predened class object. The object class does not have a superclass. Since every ChocoPy class (except object) inherits attributes and methods from a single superclass, this scheme is called *single inheritance*. The subclass/superclass relation on classes denes a graph. Since a class can only subclass another class dened previously, this graph is a tree with object as the root. In order to create an object o of type C, the expression C() is used. Upon execution, a new object is rst created with attributes initialized to their dened values. Then, the init method in the class C is

invoked. If init method is not dened in class C, then the inherited init method is called. The root class object has a default init method whose body is empty.

### 2.4 Type hierarchy

In ChocoPy, every class name is also a type. The basic type rule in ChocoPy is that if a method or variable expects a value of type P, then any value of type C may be used instead, provided that P is an ancestor of C in the class hierarchy. In other words, if C inherits from P, either directly or indirectly, then a C can be used wherever a P would suce. When an object of type C may be used in place of an object of type P, we say that C conforms to P or that C P (think: C is lower down in the inheritance tree). Conformance of class types is dened in terms of the inheritance graph. Let A, C, and P be types. Then conformance (i.e.) is dened as follows:

A A for all types A

if C is a subclass of P, then C P

if A C and C P, then A P

The root of the class hierarchy is the predened class object. The predened types int, bool, and str are subclasses of object. Additionally, for every type T in a ChocoPy program, there is a list type [T], which represents a list whose elements are of type T. For example, the type [int] represents a list of integers. List types are recursive: the type [[int]] represents a list whose elements are each a list of integers. List types are not related to each other by the relation. Every list type conforms to object; that is, [T] object for any type T. In addition to types that one can denote in ChocoPy programs, there are two special types: the type of None, which we denote <None>, and the type of [], which we denote as <Empty>. Because there is no way to explicitly write these type names in ChocoPy, no variable, parameter, or function return value ever has either of these types. In the type hierarchy, <None> *object*, <Empty> *object*, and as usual <None> <None> and <Empty> <Empty>, but otherwise these types are unrelated to any other type. To describe assignments and function invocation, we will need a slightly dierent relation, which we’ll call *assignment compatibility* and denote by the symbol*a*. Roughly, the idea is that we may assign or pass a quantity of type *T₁* to something of type *T₂* i *T₁aT₂:* More precisely, *T₁aT₂* i at least one of the following is true:

#### T₁ T₂ (i.e., ordinary subtyping).

*T₁* is <None> and *T₂* is not int, bool, or str.

*T₂* is a list type [*T*] and *T₁* is <Empty>.

*T₂* is a the list type [T] and *T₁* is [<None>], where <None>*aT*.

The last case bears mention: it is the only case in which two dierent list types are assignment compatible. It is convenient for writing such things as

#### x: [A] = [None, None]

It is rather limited, admittedly. For example,

#### x: [[A]] = [[None]]

is still invalid, although it looks perfectly sensible. There is subtle danger lurking here, and it turns out we must restrict the places where we may assign objects of type [<None>], as described under the multi-assign-stmt rule in Section 5. In some situations, we will also need to use the concept of a *join* of two or more types. The join of two types *A* and *B* is the least type *C* (using the*a*ordering) such that *A* and *B* are assignment compatible with *C*. The join operator *t* can be formally dened as follows: *C* = *A t B* if and only if:

(*AaC*) *^* (*BaC*) *^* (*8D* : (*AaD*) *^* (*BaD*)*)* (*CaD*))

That is, *C* is the join of *A* and *B* if and only if both *A* and *B* are assignment compatible with *C*, and if there exists type *D* such that *A* and *B* are assignment compatible with *D*, then *C* also is also assignment compatible with *D*. The join of any two types always exists and is unique:

If *AaB*, then *A t B* = *B t A* = *B*.

Otherwise, *A t B* is simply the *least common ancestor* of *A* and *B* in the tree-like type hierarchy dened by.

### 2.5 Values

In ChocoPy, we can have the following kinds of values.

2.5.1 Integers Integers are signed and are represented using 32 bits. The range of integers is from 2
31 to (2 31

1).
Although integers are objects, they are immutable. Arithmetic operations that cause overow lead to undened behavior in program execution.

2.5.2 Booleans There are exactly two boolean values: True and False.
2.5.3 Strings Strings are immutable sequences of characters. String literals are delimited with double quotes, e.g. "Hello World". Strings support the following three operations: retrieving the length via the len function, indexing via the s[i] syntax, and concatenation via the s1 + s2 syntax. As in Python, ChocoPy does not have a character type. Indexing into a string returns a new string of length 1. Concatenation returns a new string with length equal to the sum of the lengths of its operands.
2.5.4 Lists Lists are mutable sequences with a xed length. As such, lists in ChocoPy behave more like arrays in C. A list can be constructed using the square-brackets notation, e.g. [1, 2, 3]. Like strings, lists of type [T] support three operations: len, indexing via the lst[i] syntax, and concatenation via the lst1 + lst2 syntax. Indexing a list of type [T] returns a value of type T. Concatenation of two lists of type [T₁] and [T₂] respectively returns a new list of type [T₃], where the element type of the new list is T₃ = T₁ *t* T₂. The concatenated list has length equal to the sum of the lengths of the two operands. Additionally, lists of type [T] are mutable and support a fourth operation: element assignment via the syntax lst[i] = *f*expr*g*, where the expression on the right-hand side must conform to the type T. The rules of ChocoPy permit the construction of some rather odd (but harmless) lists with types such as [<None>] and [<Empty>]. The only variables and parameters these may be assigned to or passed as have declared type object, since it is not possible to write the specic type names [<None>] and [<Empty>] in ChocoPy. It is even possible (if not particularly useful) to index such values.
2.5.5 Objects of user-dened classes Objects are manipulated using references. That is, x = cow() implies that variable x references an object of type cow. A subsequent assignment y = x implies that x and y reference the *same* cow object in memory. The is operator can be used to determine if two expression reference the same object in memory. Objects are destroyed when they are not reachable from any local, global, or temporary variable.

2.5.6 None None is a special value that can be assigned to a variable or attribute of type object, any user-dened class type, or any list type. The is operator can be used to determine if an expression evaluates to the None value. For type-checking purposes, it has the type <None> (see section 2.4).
2.5.7 The empty list ([]) The expression [] creates a list that can be assigned to a variable having type object or any list type. For type-checking purposes, it has type <Empty>.
### 2.6 Expressions

ChocoPy supports the following categories of expressions: literals, identiers, arithmetic expressions, logical expressions, relational expressions, concatenation expressions, access expressions, and call expressions.

2.6.1 Literals and identiers The basic expression is a constant literal or a variable. Literals of type str, bool, and int have been described briey in Section 2.5, and their lexical structure is described in Section 3.4. Variables evaluate to the value contained in the variable. If an identier is bound to a global function or class, then it is not a valid expression by itself|it can appear only in specic expressions such as call expressions. This is because ChocoPy does not support rst-class functions and classes.
2.6.2 List expressions Lists may be constructed using a comma-separated sequence of expressions delimited by square brackets: [*f*expr*g*,...]. The type of a list expression containing one or more elements is [T], where T is the *least* *common ancestor* of the types of the list elements in the program’s type hierarchy. In other words, T is the least type such that the type of each element expression conforms to T. Using the the *join* operator *t* dened in Section 2.4, we can say that an expression of the form [*f*expr 1*g*, *f*expr 2*g*,..., *f*expr n*g*], where each expression *f*expr i*g* has the type T i, results in a list of type [T] where T = T 1 *t* T 2 *t*... *t* T n. The empty list expression [] has the special type <Empty>, which allows it to be assigned to (passed as) to a variable (parameter) of any list type. For example, if variable x has type [int], then the assignment x = [] is legal; x will contain an empty list of integers after this assignment.
2.6.3 Arithmetic expressions ChocoPy supports the following arithmetic expressions on two operands each of type int: *f*expr*g* + *f*expr*g*, *f*expr*g*-*f*expr*g*, *f*expr*g* * *f*expr*g*, *f*expr*g* // *f*expr*g*, and *f*expr*g* % *f*expr*g*. These operators perform integer addition, subtraction, multiplication, division quotient, and division remainder, respectively. ChocoPy does not support the *f*expr*g* / *f*expr*g* expression, which in Python evaluates to a float value. The unary expression -*f*expr*g* evaluates to the negative of the integer-valued operand. Arithmetic operations return an int value.
2.6.4 Logical expressions ChocoPy supports the following logical operations on operands of type bool: not *f*expr*g*, *f*expr*g* and *f*expr*g*, and *f*expr*g* or *f*expr*g*, which evaluate to the logical negation, conjunction, and disjunction of their operands, respectively. Logical expressions return a bool value. The binary logical expressions are also *short-circuiting*. If the left operand of an and expression evaluates to False, then a result of False is returned without evaluating the right operand at all. Similarly, if the left operand of an or expression evaluates to True, then a result of True is returned without evaluating the right operand at all. These semantics are important when the expressions in the right-hand side operands contain side-eects.

2.6.5 Relational expressions ChocoPy supports the following relational expressions on operands of type int: *f*expr*g* < *f*expr*g*, *f*expr*g* <= *f*expr*g*, *f*expr*g* > *f*expr*g*, *f*expr*g* >= *f*expr*g*. Additionally, the operands in the expressions of the form *f*expr*g* == *f*expr*g* and *f*expr*g*!= *f*expr*g* can be of types int, bool, or str, as long as both operands are of the same type. In constrast, the operands in the expressions of the form *f*expr*g* is *f*expr*g* can be the None literal or expressions of any static type other than int, bool, str. The == and != operators return true if and only if their operands evaluate to respectively equal or unequal values of integers, booleans, or strings. The is operator returns True if and only if both operands evaluate to the same object or if both operands evaluate to None.
2.6.6 Conditional expressions The expression *f*expr1*g* if *f*expr0*g* else *f*expr2*g* rst evaluates *f*expr0*g*, which must have type bool. If the result is True, then *f*expr1*g* is evaluated and its result is the value of the expression. Otherwise, *f*expr2*g* is evaluated and its value is the value of the expression.
2.6.7 Concatenation expressions The expression *f*expr*g* + *f*expr*g* can be used to concatenate two strings or two lists; the result is a new string or list, respectively.
2.6.8 Access expressions An attribute of an object can be accessed using the dot operator: *f*expr*g*.*f*id*g*. For example, x.y.z returns the value stored in the attribute z of the object obtained by evaluating the expression x.y. An element of a string or list can be accessed using the index operator: *f*expr*g*[*f*expr*g*]. For example, "Hello"[2+2] returns the string "o". Accessing a string or list x with an index i such that i < 0 or i >= len(x) aborts the program with an appropriate error message.
2.6.9 Call expressions
A call expression is of the form *f*id*g*(*f*expr*g*,...), where *f*expr*g*,... is a comma-separated list of zero or
more expressions provided as arguments to the call. If the identier is bound to a globally declared function, the expression evaluates to the result of the function call. If the identier is bound to a class, the expression results in the construction of a new object of that class, whose init method is invoked with the provided arguments. An expression of the form *f*expr*g*.*f*id*g*(*f*expr*g*,...) invokes a method with name *f*id*g* on the object returned by evaluating the expression to the left of the dot operator. The rst argument is implicit and is the object whose method is being invoked; the remaining arguments are explicitly provided in parentheses. Methods are invoked using dynamic dispatch: if the dynamic type of the object, i.e. the type at the time of execution, is T, then the method *f*id*g* dened in T or inherited by T is invoked.

### 2.7 Type annotations

In ChocoPy, static type annotations are used to explicitly provide types for variables, attributes, formal parameters and return types of functions and methods. A type annotation can either refer to a class type T, or a list type [T] such that T is the type annotation corresponding to the type of the elements of the list. Class-type annotations can be provided in one of two forms: as identiers or as string literals containing the name of a class. In ChocoPy, one can use either of the two forms for annotations. However, in Python, the former form cannot be used to refer to a class type that has not yet been dened, because Python is interpreted line by line. Since we want ChocoPy to behave similarly as Python, we will use the latter form of annotation in the above described scenario. In particular, string literals are always needed in type annotations for the rst formal parameter in method denitions, since the type of that parameter is always the same as the enclosing class, which is not yet fully dened.

### 2.8 Statements

2.8.1 Expression statements The simplest statement is a standalone expression. The expression is evaluated and its result is discarded. These types of statements are useful when they have side-eects, e.g. print("Hello").
2.8.2 Compound statements: conditionals and loops ChocoPy supports the Python-like if-elif-else syntax for conditional control-ow, with elifs and else being optional: The following code: if {expr1}: {body1} elif {expr2}: {body2} elif {expr3}: {body3}
#### is equivalent to:

if {expr1}: {body1} else: if {expr2}: {body2} else: if {expr3}: {body3}

The expressions in the if and elif conditions must have type bool. The body immediately following an if or elif condition is only evaluated if the expression evaluates to True. If the expression evaluates to False, then subsequent elif or else blocks are considered. The body following the else arm is only evaluated if all of the preceding condition expressions evaluate to False. ChocoPy supports two types of loops: simple while loops and for loops over lists and strings. while loops have the following structure:

while {expr}: {body}

The expression must be of type bool. The body is repeatedly evaluated as long as the expression evaluates to True between iterations. for loops can be used to iterate over elements of a list or characters of a string. They take the following form:

for x in {expr}: {body}

for loops are syntactic sugar; the structure above is equivalent to the following de-sugaring:

itr = {expr} idx = 0 while idx < len(itr): x = itr[idx] {body} idx = idx + 1

where len is the predened *length* function, and itr/idx are temporary variables that are not dened in the original scope. A for loop does not create new declarations for the loop variable (x in the above example); the loop variable must be declared before the for statement.

2.8.3 Assignment statements An assignment statement can be one of the following three forms: (1) *f*id*g* = *f*expr*g* assigns a value to the variable bound to the identier *f*id*g*, (2) *f*expr*g*.*f*id*g* = *f*expr*g* assigns a value to an attribute of an object, and (3) *f*expr*g*[*f*expr*g*] = *f*expr*g* assigns a value to an element of a list. When assigning a value to index i of a list x, if i < 0 or i >= len(x) then the program aborts after printing an appropriate error message. A single assignment may assign the same value to several dierent destinations. For example, the code x = y.f = z[0] = 1 assigns the integer value 1 to three memory locations: (1) the variable x, (2) the attribute f of the object referenced by variable y, (3) and the rst element of the list z, in that order. That is, the nal expression (the *right-hand side*) is evaluated rst. The result is then assigned to the *left-hand* *sides* (left of the = symbols, that is), evaluating these from left to right.
2.8.4 Pass statement The pass statement is a no-op. The program state does not change and control ow simply continues on to the next statement.
2.8.5 Return statement The return statement terminates the execution of a function and optionally returns a value using the return *f*expr*g* syntax. If a return value is not specied, then the None value is returned. It is illegal for a return statement to occur at the top level outside a function or method body. During a function’s execution, if control ow reaches the end of the function body without encountering a return statement, then the None value is implicitly returned. Consider the following example: def bar(x: int)-> object: if x > 0: return elif x == 0: return None else: pass In function bar, the execution of the function can terminate either because (1) x > 0 and a return statement with no return value is executed, or (2) x == 0 and an explicit return None is executed, or (3) x < 0 and the control ow reaches the end of the function, implicitly returning None. In functions or methods that declare a return type of int, str, or bool, all execution paths must contain a return statement with an expression that is not a None literal. In class denitions, init methods must have an empty return type (indicating that they always return None).
2.8.6 Predened classes and functions The functions print, input and len are provided by the runtime. print takes an argument of type object, and outputs outputs its printed form to the standard output, returning the value None. The valid arguments are restricted to str, int, or bool. Other arguments cause the program to abort with an error message. input takes no arguments and returns a value of type str by reading a line of input from the standard input, including the nal newline. It returns an empty string when the standard input is exhausted. len takes an argument x of type object, returning its length if it is a str or a list. Other arguments cause the program to abort with an error message. The predened classes object, int, bool, and str each dene an init method. \Calling" these classes yield an empty object, the value 0, the value False, and the value "" (empty string) respectively.

## 3 Lexical structure

This section describes the details required to implement a lexical analysis for ChocoPy. A lexical analysis reads an input le and produces a sequences of *tokens*. Tokens are matched in the input string using lexical rules that are expressed using regular expressions. Where ambiguity exists, a token comprises the longest possible string that forms a legal token, when read from left to right. The following categories of tokens exist: line structure, identiers, keywords, literals, operators, and delimiters.

### 3.1 Line structure

In ChocoPy, like in Python, whitespace may be signicant both for terminating a statement and for reasoning about the indentation level of a program statement. To accommodate this, ChocoPy denes three lexical tokens that are derived from whitespace: NEWLINE, INDENT, and DEDENT. The rules for when such tokens are generated are described next using the concepts of physical and logical lines.

3.1.1 Physical lines A physical line is a sequence of characters terminated by an end-of-line sequence. In source les and strings, the following line termination sequences can be used: the Unix form using ASCII LF (*n*n), the Windows form using the sequence ASCII CR LF (*n*r*n*n), or the old Macintosh form using the ASCII CR (*n*r) character. All of these forms can be used equally, regardless of platform. The end of input also serves as an implicit terminator for the nal physical line.
3.1.2 Logical lines A logical line is a physical line that contains at least one token that is not whitespace or comments. The end of a logical line is represented by the lexical token NEWLINE. Statements cannot cross logical line boundaries except where NEWLINE is allowed by the syntax (e.g., between statements in control-ow structures such as while loops).
3.1.3 Comments A comment starts with a hash character (#) that is not part of a string literal, and ends at the end of the physical line. Comments are ignored by the lexical analyzer; they are not emitted as tokens.
3.1.4 Blank lines A physical line that contains only spaces, tabs, and possibly a comment, is ignored (i.e., no NEWLINE token is generated).
3.1.5 Indentation The description of indentation is borrowed from the Python 3 documentation¹. \Leading whitespace (spaces and tabs) at the beginning of a logical line is used to compute the indentation level of the line, which in turn is used to determine the grouping of statements. Tabs are replaced (from left to right) by one to eight spaces such that the total number of characters up to and including the replacement is a multiple of eight (this is intended to be the same rule as used by Unix). The total number of spaces preceding the rst non-blank character then determines the line’s indentation." \The indentation levels of consecutive lines are used to generate INDENT and DEDENT tokens, using a stack, as follows: Before the rst line of the input program is read, a single zero is pushed on the stack; this will never be popped o again. The numbers pushed on the stack will always be strictly increasing from bottom to top. At the beginning of each logical line, the line’s indentation level is compared to the top of the stack. If it is equal, nothing happens. If it is larger, it is pushed on the stack, and one INDENT token is 1 [https://docs.python.org/3/reference/lexical_analysis.html](https://docs.python.org/3/reference/lexical_analysis.html)

generated. If it is smaller, it must be one of the numbers occurring on the stack; all numbers on the stack that are larger are popped o, and for each number popped o a DEDENT token is generated. At the end of the input program, a DEDENT token is generated for each number remaining on the stack that is larger than zero."

3.1.6 Whitespace between tokens Except at the beginning of a logical line or in string literals, the whitespace characters space and tab can be used interchangeably to separate tokens. Whitespace is needed between two tokens only if their concatenation could otherwise be interpreted as a dierent token (e.g., ab is one token, but a b is two tokens). Whitespace characters are not tokens; they are simply ignored.
### 3.2 Identiers

Identiers are dened as a contiguous sequence of characters containing the uppercase and lowercase letters A through Z, the underscore and, except for the rst character, the digits 0 through 9.

### 3.3 Keywords

The following strings are not recognized as identiers, and are instead recognized as distinct keyword tokens: False, None, True, and, as, assert, async, await, break, class, continue, def, del, elif, else, except, finally, for, from, global, if, import, in, is, lambda, nonlocal, not, or, pass, raise, return, try, while, with, yield. Not all keywords have special meaning in ChocoPy. For example, ChocoPy does not support async or await. However, ChocoPy uses the same list of keywords as Python in order to avoid cases where an identier is legal in ChocoPy but not in Python. Consequently, some keywords (such as async) do not appear anywhere in the grammar and will simply lead to a syntax error. An identier may contain a keyword as a substring; for example, classic is a valid identier even though it contains the substring class. This follows from the longest match rule.

### 3.4 Literals

String and integer literals are matched at the lexical analysis stage and are represented by string-valued and integer-valued tokens, respectively. The structure of these literals is described below. Boolean literals True and False are represented simply by their keyword tokens.

3.4.1 String literals String literals in ChocoPy are greatly simplied from that in Python. In ChocoPy, string literals are simply a sequence of ASCII characters delimited by (and including) double quotes: "...". The ASCII characters must lie within the decimal range 32-126 inclusive|that is, higher than or equal to the *space* character and up to *tilde*. The string itself may contain double quotes escaped by a preceding backslash, e.g. *n*". Because string literals are used both for values and for type names, it is convenient to distinguish two categories: string literals whose content has the syntax of an identier (IDSTRING in the syntax), and other string literals (denoted STRING). The value of a string token is the sequence of characters between the delimiting double quotes, with any escape sequences applied. The following escape sequences are recognized: *n*", *n*n, *n*t, *nn*, which correspond to a literal double quote, a newline, a tab, and a literal backslash respectively. Any other escape sequence is considered illegal. Some examples follow: Literal Value "Hello" Hello "He*n*"ll*n*"o" He"ll"o "He*nnn*"llo" He*n*"llo "Hell*n*o" (error: "*n*o" not recognized)

3.4.2 Integer literals Integer literals in ChocoPy are composed of a sequence of one or more digits 0-9, where the leftmost digit may only be 0 if it is the only character in the sequence. That is, non-zero valued integer literals may not have leading zeros. The integer value of such literals is interpreted in base 10. The maximum interpreted value can be 2
31 1 for the literal 2147483647. A literal with a larger value than this limit results in a lexical error.

### 3.5 Operators and delimiters

The following is a space-separated list of symbols that correspond to distinct ChocoPy tokens: +-* // % < > <= >= ==!= = () [], :.->

## 4 Syntax

Figure 3 lists the grammar of ChocoPy using an extended BNF notation. Keyword tokens are represented

in a boldfaced font. Literals and whitespace tokens are represented in UPPERCASE. Nonterminals are formatted *lowercase italics*. Operators and delimiters are formatted as-is. The notation J*: : :* K is used to group one or more symbols in a production rule and are not tokens in the input language. Symbols or groups may be annotated as follows: ‘?’ denotes that the preceding symbol or group is optional, ‘ ’ denotes zero or more repeating occurrences and ‘+’ denotes one or more repeating occurrences. The puzzling division of expressions into *expr* and *cexpr* captures the obscure point that ChocoPy (like Python) only allows logical binary or unary expressions as operands of logical operators (and, or, not). As a result, the expression True == not False is supposed to produce a syntax error (the correct expression being True == (not False)).

### 4.1 Precedence and Associativity

Operators in ChocoPy have the same precedence as they do in Python. The following table summarizes the precedence of operators in ChocoPy, from lowest precedence (least binding) to highest precedence (most binding).

|Precedence|Operator(s)|Associativity|
|---|---|---|
||1 if else 2 or 3 and 4 not 5 ==, !=, <, >, <=, >=, is 6 +, - (binary) 7 *, //, % 8 - (unary) 9 ., []|Right Left Left N/A None Left Left N/A Left|

Note that the comparison operators are nonassociative (so ChocoPy, unlike Python 3, does not allow expressions such as x < y < z).

*program* ::= J*var def j func def j class def* K *stmt* *class def* ::= class ID ( ID ) : NEWLINE INDENT *class body* DEDENT *class body* ::= pass NEWLINE *j* J*var def j func def* K +

*func def* ::= def ID ( J*typed var* J, *typed var*K K ? ) J-> typeK ? : NEWLINE INDENT *func body* DEDENT *func body* ::= J*global decl j nonlocal decl j var def j func def* K *stmt* +

*typed var* ::= ID : *type* *type* ::= ID *j* IDSTRING *j* [ *type*] *global decl* ::= global ID NEWLINE *nonlocal decl* ::= nonlocal ID NEWLINE *var def* ::= *typed var* = *literal* NEWLINE *stmt* ::= *simple stmt* NEWLINE *j* if *expr* : *block* Jelif *expr* : *block* K Jelse : *block*K ?

*j* while *expr* : *block* *j* for ID in *expr* : *block* *simple stmt* ::= pass *j expr* *j* return J*expr*K ?

*j* J *target* = K + *expr* *block* ::= NEWLINE INDENT *stmt* + DEDENT *literal* ::= None *j* True *j* False *j* INTEGER *j* IDSTRING *j* STRING *expr* ::= *cexpr* *j* not *expr* *j expr* Jand *j* orK *expr* *j expr* if *expr* else *expr* *cexpr* ::= ID *j literal* *j* [ J*expr* J, *expr*K K ?] *j* ( *expr*) *j member expr* *j index expr* *j member expr* ( J*expr* J, *expr*K K ? ) *j* ID ( J*expr* J, *expr*K K ? ) *j cexpr bin op cexpr* *j*-*cexpr* *bin op* ::= + *j*-*j* * *j* // *j* % *j* == *j*!= *j* <= *j* >= *j* < *j* > *j* is *member expr* ::= *cexpr*. ID *index expr* ::= *cexpr* [ *expr*] *target* ::= ID *j member expr* *j index expr*

Figure 3: Grammar describing the syntax of the ChocoPy language.

## 5 Type rules

This section formally denes the type rules of ChocoPy. The type rules dene the type of every ChocoPy expression in a given context. The context is the type environment, which describes the type of every unbound identier appearing in an expression. The type environment is described in Section 5.1. Section

5.2 gives the type rules.
### 5.1 Type environments

To a rst approximation, type checking in ChocoPy can be thought of as a bottom-up algorithm: the type of an expression *e* is inferred from the (previously inferred) types of *e*’s sub-expression. For example, an integer 1 has type int; there are no sub-expression in this case. As another example, if the types of *e₁* and *e₂* are int, then the expression *e₁ > e₂* has type bool. A complication arises in the case of an expression *v*, where *v* is a variable or a function. It is not possible to say what the type of *v* is in a strictly bottom-up algorithm; we need to know the type declared for *v* in the larger expression. Such a declaration must exist for every variable and function in valid ChocoPy programs. To capture information about the types of identiers, we use a *type environment*. The type environment consists of four parts: *O* a local environment, *M* a method/attribute environment *M*, *C* the name of the current class in which the expression or statement appears, and *R* the return type of the function or method in which the expression or statement appears. *C* is*?* when the expression or statement appears outside a class,

i.e. as a statement or expression in the top level. Similarly, *R* is*?* when the expression or statement appears outside a function or method, i.e. as a statement or expression in the top level. The local environment and the method/attribute environment are both maps. The local environment is a function of the form:
#### O(v) = T

which assigns the type *T* to a variable *v*. The same environment also holds information about function signatures. For example,

*O*(*f*) = *fT₁ Tn! T₀*; *x₁; : : :; xn*; *v₁* : *T₁* *0* *; : : :; vm*: *Tm* *0* *g*

gives the type of *f* and denotes that identier *f* has formal parameters *x₁; : : : xn*of types *T₁; : : :; Tn*, respectively, and has return type *T₀*. The identiers *v₁; : : :; vm*are the variables and nested functions declared in the body of *f* and their types are *T₁* *0* *; : : :; Tm* *0*, respectively. The method/attribute environment similarly maps a class and its attributes and methods to their types. For example,

#### M (C; a) = T

maps the attribute *a* in the class *C* to the type *T*. Similarly,

*M* (*C; m*) = *fT₁ Tn! T₀*; *x₁; : : :; xn*; *v₁* : *T₁* *0* *; : : :; vk*: *Tk0g*

maps the method *m* of class *C* to it type. Specically, it denotes that method *m* in class *C* has formal parameters *x₁; : : : xn*of types *T₁; : : :; Tn*, respectively, and has return type *T₀*. The identiers *v₁; : : :; vk*are the variables and nested functions declared in the body of *m* and their types are *T₁* *0* *; : : :; Tk0*, respectively. The third component of the type environment is the name of the class containing the expression or statement to be type checked. The fourth component of the type environment is the return type *R* of the function or method containing the expression or statement to be type checked. When type checking function and method denitions, we need to propagate the typing environment from an outer scope to the function scope, where the binding of any identier is inherited unless the function declares a formal parameter, variable, or a nested function with the same name. Let *O* be the current local environment and *f* be a function with type denition *fT₁ Tn! T₀*; *x₁; : : :; xn*; *v₁* : *T₁* *0* *; : : :; vm*: *Tm* *0*

*g*. When type checking the denition of *f*, we type check its body using the local environ-
ment *O*[*T₁=x₁*][*T₂=x₂*]*: : :*[*Tn=xn*][*T₁* *0* *=v₁*]*: : :*[*Tm* *0* *=vm*], where the notation *O*[*T =c*] is used to construct a new mapping as follows:

*O*[*T =c*](*c*) = *T* *O*[*T =c*](*d*) = *O*(*d*) if *d 6*= *c*

### 5.2 Type checking rules

The general form of a type checking rule is:

... *O; M; C; R ‘ e* : *T*

The rule should be read: in the type environment with local environment *O*, method/attribute environment *M*, containing class *C*, and return type *R*, the expression *e* has type *T*. The line below the bar is a typing judgment: the turnstyle \*‘*" separates context (*O; M; C; R*) from a proposition *e* : *T*. The dots above the horizontal bar stand for other judgments about the types of sub-expressions of *e*. These other judgments are hypotheses of the rule; if the hypotheses are satised, then the judgment below the bar is true.

Variables. The rule for variables is simply that if the environment assigns an identier *id* a type *T*, then the expression *id* has type *T*.

<u>O(id) = T; where T is not a function type.</u> [var-read] *O; M; C; R ‘ id* : *T*

We must, however, prohibit identiers with function types when reading values (that is, when identiers are used as expressions in the syntax). This simply reects the fact that ChocoPy does not treat functions as rst-class (assignable, storable) values.

Variable Denitions and Assignments. This assignment rule|as well as others|uses the relation*a* (ref. Section 2.4). The rule says that the assigned expression *e₁* must have a type *T₁* that is assignment compatible with the type *T* of the identier *id* in the type environment.

*O*(*id*) = *T* *O; M; C; R ‘ e₁* : *T₁* *T₁aT* [var-assign-stmt] *O; M; C; R ‘ id* = *e₁*

#### Variable denitions obey a similar rule:

*O*(*id*) = *T* *O; M; C; R ‘ e₁* : *T₁* *T₁aT* [var-init] *O; M; C; R ‘ id*: *T* = *e₁*

The colon used below the line in the rule for var-init is the colon in the syntax for type annotations.

Statement and Denition Lists. These type check if all the component denitions and statements type check.

*O; M; C; R ‘ s₁* *O; M; C; R ‘ s₂* ... *O; M; C; R ‘ sn* <u>n 1</u> [stmt-def-list] *O; M; C; R ‘ s₁* NEWLINE *s₂* NEWLINE *: : : sn*NEWLINE

Pass Statements. [pass] *O; M; C; R ‘* pass

#### Expression Statements.

<u>O; M; C; R ‘ e : T</u> [expr-stmt] *O; M; C; R ‘ e*

Literals. [bool-false] *O; M; C; R ‘* False : *bool*

[bool-true] *O; M; C; R ‘* True : *bool*

<u>i is an integer literal</u> [int] *O; M; C; R ‘ i* : *int*

<u>s is a string literal</u> [str] *O; M; C; R ‘ s* : *str*

The None literal is assigned the (unmentionable) type <None>:

[none] *O; M; C; R ‘* None : <None>

#### Arithmetic and Numerical Relational Operators.

<u>O; M; C; R ‘ e : int</u> [negate] *O; M; C; R ‘*-*e* : *int*

*O; M; C; R ‘ e₁* : *int* *O; M; C; R ‘ e₂* : *int* *op 2f*+*;;; ==;*%*g* [arith] *O; M; C; R ‘ e₁op e₂* : *int*

*O; M; C; R ‘ e₁* : *int* *O; M; C; R ‘ e₂* : *int*

<u>./ 2f<;<=; >;>=;==;!=g</u>
[int-compare] *O; M; C; R ‘ e₁./ e₂* : *bool*

#### Logical Operators.

*O; M; C; R ‘ e₁* : *bool* *O; M; C; R ‘ e₂* : *bool*

*./2f*==*;*!=*g*
[bool-compare] *O; M; C; R ‘ e₁./ e₂* : *bool*

*O; M; C; R ‘ e₁* : *bool* *O; M; C; R ‘ e₂* : *bool* [and] *O; M; C; R ‘ e₁* and *e₂* : *bool*

*O; M; C; R ‘ e₁* : *bool* *O; M; C; R ‘ e₂* : *bool* [or] *O; M; C; R ‘ e₁* or *e₂* : *bool*

*O; M; C; R ‘ e* : *bool* [not] *O; M; C; R ‘* not *e* : *bool*

#### Conditional Expressions.

*O; M; C; R ‘ e₀* : *bool* *O; M; C; R ‘ e₁* : *T₁* *O; M; C; R ‘ e₂* : *T₂* [cond] *O; M; C; R ‘ e₁* if *e₀* else *e₂* : *T₁ t T₂*)

#### String Operations.

*O; M; C; R ‘ e₁* : *str* *O; M; C; R ‘ e₂* : *str*

*./ 2f*==*;*!=*g*
[str-compare] *O; M; C; R ‘ e₁./ e₂* : *bool*

*O; M; C; R ‘ e₁* : *str* *O; M; C; R ‘ e₂* : *str* [str-concat] *O; M; C; R ‘ e₁* + *e₂* : *str*

*O; M; C; R ‘ e₁* : *str* <u>O; M; C; R ‘ e₂ : int</u> [str-select] *O; M; C; R ‘ e₁*[*e₂*] : *str*

#### The ‘is’ Operator.

*O; M; C; R ‘ e₁* : *T₁* *O; M; C; R ‘ e₂* : *T₂* <u>T₁; T₂ are not one of int; str; bool</u> [is] *O; M; C; R ‘ e₁* is *e₂* : *bool*

Object Construction. If *T* is the name of a class, then object-construction expressions of that class can be typed as follows:

*T* is a class [new] *O; M; C; R ‘ T*() : *T*

#### List Displays.

*n* 1 *O; M; C; R ‘ e₁* : *T₁* *O; M; C; R ‘ e₂* : *T₂* ... *O; M; C; R ‘ en*: *Tn* *T* = *T₁ t T₂ t : : : t Tn* [list-display] *O; M; C; R ‘* [*e₁; e₂; : : :; en*] : [*T*]

The empty list is a special case:

[nil] *O; M; C; R ‘* [] : <Empty>

#### List Operators.

*O; M; C; R ‘ e₁* : [*T₁*] *O; M; C; R ‘ e₂* : [*T₂*] *T* = *T₁ t T₂* [list-concat] *O; M; C; R ‘ e₁* + *e₂* : [*T*]

*O; M; C; R ‘ e₁* : [*T*] <u>O; M; C; R ‘ e₂ : int</u> [list-select] *O; M; C; R ‘ e₁*[*e₂*] : *T*

*O; M; C; R ‘ e₁* : [*T₁*] *O; M; C; R ‘ e₂* : *int* *O; M; C; R ‘ e₃* : *T₃* *T₃aT₁* [list-assign-stmt] *O; M; C; R ‘ e₁*[*e₂*] = *e₃*

Attribute Access, Assignment, and Initialization. For attribute access, we use the class-member environment *M* :

*O; M; C; R ‘ e₀* : *T₀* *M* (*T₀; id*) = *T* [attr-read] *O; M; C; R ‘ e₀:id* : *T*

*O; M; C; R ‘ e₀* : *T₀* *O; M; C; R ‘ e₁* : *T₁* *M* (*T₀; id*) = *T* *T₁aT* [attr-assign-stmt] *O; M; C; R ‘ e₀:id* = *e₁*

*M* (*C; id*) = *T* *O; M; C; R ‘ e₁* : *T₁* <u>T₁aT</u> [attr-init] *O; M; C; R ‘ id*: *T* = *e₁*

Multiple Assignments. Multiple assignment is type-checked by decomposing into individual single assignments, as follows.

*n >* 1 *O; M; C; R ‘ e₀* : *T₀* *O; M; C; R ‘ e₁* = *e₀* ... *O; M; C; R ‘ en*= *e₀* <u>T₀</u> *6*<u>= [<None>]</u> [multi-assign-stmt] *O; M; C; R ‘ e₁* = *e₂* = = *en*= *e₀*

The restriction that *T₀ 6*= [<None>] avoids a subtle type-safety issue. It is dangerous to allow there to be two dierent views of a list with diering element types. The type [<None>] can only arise from list displays. As long as the value of such a display is immediately consumed by assignment to a single variable, parameter, or operand (+ for lists), there will be only be one opinion as to its type subsequently. But multiple assignment opens the possibility of programs like this:

x: [A] = None y: [[int]] = None x = y = [None] # Trouble ahead! x[0] = A() print(y[0][0]) #???

Java, for example, gets itself into this bind and therefore needs a runtime ArrayStoreException. To avoid it in ChocoPy, we conservatively forbid values of the type [<None>] to be multiply assigned. There is another very subtle point lurking here in the case where *e₀* has type <Empty> (the type of the empty list). In this case, however, we need no special rule because in ChocoPy, there is no .append method to allow elements to be added to an empty list. Were that not the case, we could get this situation:

A: [int] = None B: [str] = None A = B = []

A.append(3)
and we’d subsequently have B[0] returning the value 3, which is certainly not a string.

#### Function Applications.

*O; M; C; R ‘ e₁* : *T₁* *00*

*O; M; C; R ‘ e₂* : *T₂* *00* ... *O; M; C; R ‘ en*: *Tn* *00*

*n* 0 *O*(*f*) = *fT₁ Tn! T₀*; *x₁; : : :; xn*; *v₁* : *T₁* *0* *; : : :; vm*: *Tm* *0* *g* <u>81 i n : Ti</u> *00* <u>aTi</u> [invoke] *O; M; C; R ‘ f*(*e₁; e₂; : : :; en*) : *T₀*

To type check a function invocation, each of the arguments to the function must be rst type checked. The type of each argument must conform to the types of the corresponding formal parameter of the function. The invocation expression is assigned the function’s declared return type. Method dispatch expressions are type checked in a similar fashion. The key dierence from the function invocation expression is that the target method is determined by consulting the method/attribute environment using the type of the receiver object expression:

*O; M; C; R ‘ e₁* : *T₁* *00*

*O; M; C; R ‘ e₂* : *T₂* *00* ... *O; M; C; R ‘ en*: *Tn* *00*

*n* 1 *M* (*T₁* *00* *; f*) = *fT₁ Tn! T₀*; *x₁; : : :; xn*; *v₁* : *T₁* *0* *; : : :; vm*: *Tm* *0* *g* *T₁* *00* *aT₁* <u>8i: 2 i n : Ti</u> *00* <u>aTi</u> [dispatch] *O; M; C; R ‘ e₁:f*(*e₂; : : :; en*) : *T₀*

Return Statements. This is where the return-type environment comes into play:

*O; M; C; R ‘ e* : *T* *TaR* [return-e] *O; M; C; R ‘* return *e*

<None>*aR* [return] *O; M; C; R ‘* return

#### Conditional Statements.

*O; M; C; R ‘ e₀* : *bool* *O; M; C; R ‘ b₀* *O; M; C; R ‘ e₁* : *bool* *O; M; C; R ‘ b₁* ... *O; M; C; R ‘ en*: *bool* *O; M; C; R ‘ bn* *n* 0 <u>O; M; C; R ‘ bn+1</u> [if-elif-else] *O; M; C; R ‘* if *e₀*: *b₀* elif *e₁*: *b₁ : : :* elif *en*: *bn*else: *bn*+1

#### While Statements.

*O; M; C; R ‘ e* : *bool* *O; M; C; R ‘ b* [while] *O; M; C; R ‘* while *e*: *b*

#### For Statements.

*O; M; C; R ‘ e* : *str* *O*(*id*) = *T* *straT* *O; M; C; R ‘ b* [for-str] *O; M; C; R ‘* for *id* in *e*: *b*

*O; M; C; R ‘ e* : [*T₁*] *O*(*id*) = *T* *T₁aT* <u>O; M; C; R ‘ b</u> [for-list] *O; M; C; R ‘* for *id* in *e*: *b*

Function Denitions. To type a function denition for *f*, we check the body of the function *f* in an environment where *O* is extended with bindings for the names explicitly declared by *f*.

*T ;*0if -> is present, *T* = <None>*; otherwise:* *0 0* *O*(*f*) = *fT₁ Tn! T*; *x₁; : : :; xn*; *v₁* : *T₁; : : :; vm*: *Tmg* *n* 0 *m* 0 *0 0* <u>O[T₁=x₁]: : :[Tn=xn][T₁=v₁]: : :[Tm=vm]; M; C; T ‘ b</u> [func-def] ? *O; M; C; R ‘* def *f* (*x₁*: *T₁; : : :; xn*: *Tn*) J-> *T₀*K : *b*

#### Likewise for methods:

*T ;*0if -> is present, *T* = <None>*; otherwise:* *0 0* *M* (*C; f*) = *fT₁ Tn! T*; *x₁; : : :; xn*; *v₁* : *T₁; : : :; vm*: *Tmg* *n* 1 *m* 0 *C* = *T₁* *0 0* <u>O[T₁=x₁]: : :[Tn=xn][T₁=v₁]: : :[Tm=vm]; M; C; T ‘ b</u> [method-def] ? *O; M; C; R ‘* def *f* (*x₁*: *T₁; : : :; xn*: *Tn*) J-> *T₀*K : *b*

Class Denitions. Class denitions are type checked by propagating the appropriate typing environment:

<u>O; M; C; R ‘ b</u> [class-def] *O; M; ?; R ‘* class *C*(*S*): *b*

The Global Typing Environment. The following functions and class methods are predened globally:

*O*(*len*) = *fobject ! int*; *argg* *O*(*print*) = *fobject !* <None>; *argg* *O*(*input*) = *f! strg*

|M (object;|init|) = fobject ! <None>; selfg|
|---|---|---|
|M (str;|init|) = fobject ! <None>; selfg|
|M (int;|init|) = fobject ! <None>; selfg|
|M (bool;|init|) = fobject ! <None>; selfg|

## 6 Operational semantics

This section contains the formal operational semantics for the ChocoPy language. The operational semantics dene how every denition, statement, or expression in a ChocoPy program should be evaluated in a given context. The context has four components: a global environment, a local environment, a store, and a return

value. Section 6.1 describes these components. Section 6.2 denes the syntax used to refer to ChocoPy values, and Section 6.3 denes the syntax used to refer to class denitions. Keep in mind that a formal semantics is a specication only|it does not describe an implementation. The purpose of presenting the formal semantics is to make clear all the details of the behavior of a ChocoPy program. How this behavior is implemented is another matter.

### 6.1 Evaluation context

The value of a ChocoPy expression depends on the context in which it is evaluated. The context comprises of an *environment*, which maps variable identiers to *locations*. Intuitively, an environment tells us for a given identier the address of the memory location where that identier’s value is stored. For a given expression, the environment must assign a location to all identiers to which the expression may refer. For the expression *a* + *b*, we need an environment that maps *a* to some location and *b* to some location. We’ll use the following syntax to describe environments, which is very similar to the syntax of type environments dened in Section 5.1.

*E* = [*a* : *l₁; b* : *l₂*]

This environment maps variable *a* to location *l₁* and variable *b* to location *l₂*. The second component of the evaluation context is the *store* (memory). The store maps locations to values, where values in ChocoPy are objects or functions. Intuitively, a store tells us what value is stored in a given memory location. For the moment, assume all values are integers. A store is similar to an environment:

*S* = [*l₁ 7!* 42*; l₂ 7!* 7]

This store maps location *l₁* to the value 42 and the location *l₂* to the value 7. Given an environment and a store, the value of a variable can be retrieved by rst looking up the location of a variable in the environment *E*, and then looking up the value stored at this location in the store *S*. For example, the value of variable *a* can be looked up in the following way:

*E*(*a*) = *l₁* *S*(*l₁*) = 42

Together, the environment and the store dene the execution state at a particular step of the evaluation of a ChocoPy program. The double indirection from identiers to locations to values allows us to model variables. Consider what happens if the value 11 is assigned to variable *a* in the environment and store dened above. Assigning to a variable means changing the value to which it refers but not its location. To perform the assignment, we look up the location for *a* in the environment E and then change the mapping for the obtained location to the new value, giving a new store *S⁰*.

*E*(*a*) = *l₁* *S⁰* = *S*[11*=l₁*]

Where the syntax *S⁰* = *S*[*v=l*] denotes a new store *S⁰* that is identical to the store *S*, except that *S⁰* maps location *l* to value *v*. It is formally dened as follows:

*S*[*v=l*](*l*) = *v* *S*[*v=l*](*l⁰*) = *S*(*l⁰*) if *l⁰ 6*= *l*

There are also situations in which the environment is modied. Consider the denition of a variable in ChocoPy:

#### x : int = 36

When evaluating this denition, we must introduce a new identier *x* into the environment, which will be valid for the rest of the scope in which denition occurs. If the current environment and store are *E* and *S* respectively, then the new environment and store after evaluating this denition are *E⁰* and *S⁰* respectively, which are dened by:

*l* *x*= *newloc*(*S*) *E⁰* = *E*[*lx=x*] *S⁰* = *S*[36*=lx*]

The rst step is to allocate a location for the variable *x*. The location should be fresh, meaning that the current store should not have a mapping for it. The function *newloc* applied to a store gives us an unused location in that store. We then create a new environment *E⁰* that maps *x* to *lx*but also contains all of the mappings of *E* for identiers other than *x*. If *x* already has a mapping in *E*, the new environment *E⁰* hides this old mapping. We must also update the store to map the new location to a value. In this case *lx*maps to the value 36, which is the initial value for *x* specied in the variable’s denition. When we need several distinct new locations at once, as when creating a new list, we’ll extend *newloc* to do so: *newloc*(*S; n*) produces *n* distinct new locations in *S*. The example in this subsection oversimplies ChocoPy environments and stores a bit, because simple integers are not ChocoPy values. Even integers are full-edged objects in ChocoPy. In ChocoPy, the evaluation context consists of an additional component: a global environment *G*. *G* is an environment similar to *E*, but it always contains mappings for variables and functions dened at the global scope. This environment is useful for handling cases where a nested function references a global variable x via the global x declaration, bypassing any inherited mappings for variable x from the enclosing function.z

### 6.2 Syntax for values

To describe ChocoPy semantics, we use the following categories of values: objects that are instances of classes, list objects, the None value, and functions (or methods).

6.2.1 Class instances Let *v* be a value corresponding to a object belonging to class *X*. The value *v* is represented by the syntax:
*v* = *X*(*a₁* = *l₁; a₂* = *l₂; : : :; an*= *ln*)

where each *ai*for 1 *i n* is an attribute or method (including those inherited) of class *X*, and each *li*is the location where the value of attribute or method *ai*is stored. Each *ai*is distinct in a semantically valid ChocoPy program. For base objects of ChocoPy (i.e., int, str, bool), we use a special case of the above syntax. Base objects have a class name, but their attributes are not like attributes of normal classes, because they cannot be modied. Therefore, we describe base objects using the following syntax:

*int*(5) *bool*(*T rue*) *str*(7*;*\ChocoPy")

Integers and booleans contain just one component: the integer or boolean value respectively. Strings contain two components: a length and the actual sequence of ASCII characters.

6.2.2 List objects A list object *v* of length *n* is represented by the syntax:
*v* = [*l₀; l₂; : : :; ln* 1]

where each *li*is the element at index *i*.

6.2.3 None The special value None is represented simply as *None*.
6.2.4 Functions Functions are not rst-class values in ChocoPy; that is, ChocoPy variables cannot store references to functions and ChocoPy expressions cannot evaluate to a function. However, functions are represented as values in the formal semantics and are the result of evaluating function and method denitions. Global functions, nested functions, and methods of classes are all represented by the same syntax, which is as follows:
*v* = (*x₁; : : :; xn; y₁* = *e₁; : : :; yk*= *ek; bbody; Ef*)

Here, *v* is a function having *n* formal parameters *x₁; : : :; xn*and *k* local denitions *y₁; : : :; yk*. Local denitions include local variables and nested functions. The term *ei*represents the denition corresponding to the identier *yi*. For variable denitions, *ei*is the literal expression that gives its initial value. For nested functions, *ei*is the function denition itself. The term *bbody*is the function’s body, which is a sequence of statements. The term *Ef*is the environment in which the function is dened, with appropriate substitutions for global declarations within the function. Certain functions are predened in ChocoPy: len, print, and input. They don’t have bodies|at least not bodies written in ChocoPy|but if we are going to denote their values, we’ll need some way of denoting these bodies. So, for the predened functions, we’ll use the \bodies" native print, native len, and native input, suggestive of the syntax used for native methods in Java.

### 6.3 Syntax for class denitions

When referring to class denitions, we need to use an additional notation. The mapping *class* is used to get the attributes and methods dened in a particular class. The syntax is as follows:

*class*(*A*) = (*a₁* = *e₁; : : :; am*= *em*)

where each *ai*is an attribute or method belonging to the class *A* (including inherited members). If *ai*is an attribute, then the corresponding *ei*is the literal expression that species the attribute’s initial value. If *ai* is a method name, then *ei*is the corresponding method denition (which has the same syntax as a function denition). For classes that inherit the method init from its denition in class object, we assume that the method body contains a single statement: pass.

### 6.4 Operational rules

Equipped with the notation for environments, stores, and the syntax for values and class denitions, we can now present the formal operational semantics rules for ChocoPy. The general form of a rule is:

.. <u>.</u> *G; E; S ‘ e* : *v; S⁰; R⁰*

This rule should be read as follows: in a context with global environment *G*, local environment *E*, and store *S*, the program fragment *e* evaluates to value *v*, after which the new store is *S⁰* and the returned value is *R⁰*.

Program fragments include expressions, denitions, statements, and sequences of statements. The value *v* will only be meaningful when *e* is an expression or function/method denition; in other cases, we represent the value *v* by the ‘ ’ (underscore) symbol to denote the absence of any value.

Literals.

[none] *G; E; S ‘* None : *None; S;*

[bool-false] *G; E; S ‘* False : *bool*(*f alse*)*; S;*

[bool-true] *G; E; S ‘* True : *bool*(*true*)*; S;*

<u>i is an integer literal</u> [int] *G; E; S ‘ i* : *int*(*i*)*; S;*

*s* is a string literal <u>n is the length of the string s</u> [str] *G; E; S ‘ s* : *str*(*n; s*)*; S;*

#### Pass Statements.

[pass] *G; E; S ‘* pass :*; S;*

Expression Statements. Here, we evaluate an expression, which may modify the store, and then discard its value:

<u>G; E; S ‘ e : v; S⁰;</u> [expr-stmt] *0* *G; E; S ‘ e* :*; S;*

Variable Accesses. The values of variables involve nding their location in the environment and then reading or writing their values in the store.

*E*(*id*) = *lid* *S*(*lid*) = *v* [var-read] *G; E; S ‘ id* : *v; S;*

*G; E; S ‘ e* : *v; S₁;* *E*(*id*) = *lid* *S₂* = *S₁*[*v=lid*] [var-assign-stmt] *G; E; S ‘ id* = *e* :*; S₂;*

The last rule should be read as follows: the right-hand side of the assignment is rst evaluated with the context *G; E; S;* to produce value *v*; this evaluation may result in a modied store *S₁*. After evaluating the variable assignment, the new store is *S₂*, which is a modication of *S₁* where the location *lid*maps to the evaluated value of the right-hand-side: *v*.

Numerical Operations. These relate ChocoPy operators to corresponding mathematical operators.

*G; E; S ‘ e* : *int*(*i₁*)*; S₁;* *v* = *int*( *i₁*) [negate] *G; E; S ‘*-*e* : *v; S₁;*

*G; E; S ‘ e₁* : *int*(*i₁*)*; S₁;* *G; E; S₁ ‘ e₂* : *int*(*i₂*)*; S₂;* *op 2f*+*;*-*;* **;*//*;* %*g* *op 2f*//*;* %*g) i₂ 6*= 0 *v* = *int*(*i₁ op i₂*) [arith] *G; E; S ‘ e₁ op e₂* : *v; S₂;*

*G; E; S ‘ e₁* : *int*(*i₁*)*; S₁;* *G; E; S₁ ‘ e₂* : *int*(*i₂*)*; S₂;*

*./ 2f;*<=*;* >*;*>=*;*==*;*!=*g* (< *bool*(*true*) if *i₁./ i₂* *v* = *bool*(*f alse*) otherwise
[int-compare] *G; E; S ‘ e₁./ e₂* : *v; S₂;*

*G; E; S ‘ e₁* : *bool*(*b₁*)*; S₁;* *G; E; S₁ ‘ e₂* : *bool*(*b₂*)*; S₂;*

*./ 2f;*!=*g* (== *bool*(*true*) if *b₁./ b₂* *v* = *bool*(*f alse*) otherwise
[bool-compare] *G; E; S ‘ e₁./ e₂* : *v; S₂;*

#### String Operations.

*G; E; S ‘ e₁* : *str*(*n₁; s₁*)*; S₁;* *G; E; S₁ ‘ e₂* : *str*(*n₂; s₂*)*; S₂;*

*./ 2f;*!=*g* 8== ><*bool*(*true*) if*./* is = and *s₁* = *s₂* *v* = *bool*(*true*) if*./* is != and *s₁ 6*= *s₂* >: *bool*(*f alse*) otherwise
[str-compare] *G; E; S ‘ e₁./ e₂* : *v; S₂;*

*G; E; S ‘ e₁* : *str*(*n₁; s₁*)*; S₁;* *G; E; S₁ ‘ e₂* : *str*(*n₂; s₂*)*; S₂;* <u>v = str(n₁ + n₂; s₁:s₂) where s₁:s₂ is the concatenated string</u> [str-concat] *G; E; S ‘ e₁* + *e₂* : *v; S₂;*

*G; E; S ‘ e₁* : *str*(*n; c₁:c₂: : :cn*)*; S₁;* *G; E; S₁ ‘ e₂* : *int*(*i*)*; S₂;* 0 *i < n* *v* = *str*(1*; ci*+1) [str-select] *G; E; S ‘ e₁*[*e₂*] : *v; S₂;*

#### Object Identity.

*G; E; S ‘ e₁* : *v₁; S₁;* *G; E; S* 81 *‘ e₂* : *v₂; S₂;* > <*bool*(*true*) if *v₁* = *None* and *v₂* = *None* *v* = *bool*(*true*) if *v₁* and *v₂* are the same object in memory > : <u>bool(f alse) otherwise</u> [is] *G; E; S ‘ e₁* is *e₂* : *v; S₂;*

Logical Operators. The rule for the unary logical operator not is simple: the operand’s value is negated. The binary logical operators perform short-circuit evaluation: if the result of logical conjunction or disjunction is apparent from evaluating the rst operand (because the operand corresponds to False or True respectively), then the second operand is not evaluated at all. Otherwise, the result of the conjunction or disjunction is equal to the value of the second operand.

*G; E; S* ( *‘ e* : *bool*(*b*)*; S₁;* *bool*(*f alse*) if *b* = *true* *v* = *bool*(*true*) otherwise [not] *G; E; S ‘* not *e* : *v; S₁;*

*G; E; S ‘ e₁* : *bool*(*f alse*)*; S₁;* [and-1] *G; E; S ‘ e₁* and *e₂* : *bool*(*f alse*)*; S₁;*

*G; E; S ‘ e₁* : *bool*(*true*)*; S₁;* *G; E; S₁ ‘ e₂* : *v; S₂;* [and-2] *G; E; S ‘ e₁* and *e₂* : *v; S₂;*

*G; E; S ‘ e₁* : *bool*(*true*)*; S₁;* [or-1] *G; E; S ‘ e₁* or *e₂* : *bool*(*true*)*; S₁;*

*G; E; S ‘ e₁* : *bool*(*f alse*)*; S₁;* *G; E; S₁ ‘ e₂* : *v; S₂;* [or-2] *G; E; S ‘ e₁* or *e₂* : *v; S₂;*

Conditional Statements and Expressions. conditional (if-else and if-elif-else) statements and conditional expressions are evaluated by rst evaluating the condition, and then deciding which branch to evaluate. The body of a conditional statement may contain a return statement, which is propagated by the enclosing if statement as well.

*G; E; S ‘ e* : *bool*(*true*)*; S₁;* *G; E; S₁ ‘ b₁* :*; S₂; R* [if-else-true] *G; E; S ‘* if *e*: *b₁* else: *b₂* :*; S₂; R*

*G; E; S ‘ e* : *bool*(*f alse*)*; S₁;* *G; E; S₁ ‘ b₂* :*; S₂; R* [if-else-false] *G; E; S ‘* if *e*: *b₁* else: *b₂* :*; S₂; R*

*G; E; S ‘ e₀* : *bool*(*true*)*; S₁;* <u>G; E; S₁ ‘ b₀ :; S₂; R</u> [if-elif-true] *G; E; S ‘* if *e₀*: *b₀* elif *e₁*: *b₁ : : :* elif *en*: *bn*else: *bn*+1:*; S₂; R*

*G; E; S ‘ e₀* : *bool*(*f alse*)*; S₁;* <u>G; E; S₁ ‘ if e₁: b₁ : : : elif en: bnelse: bn+1:; S₂; R</u> [if-elif-false] *G; E; S ‘* if *e₀*: *b₀* elif *e₁*: *b₁ : : :* elif *en*: *bn*else: *bn*+1:*; S₂; R*

<u>G; E; S ‘ if e: b₁ else: pass :; S₁; R</u> [if-no-else] *G; E; S ‘* if *e*: *b₁* :*; S₁; R*

*G; E; S ‘ e* : *bool*(*true*)*; S₁;* *G; E; S₁ ‘ b₁* : *v; S₂;* [if-else-expr-true] *G; E; S ‘ b₁* if *e* else *b₂* : *v; S₂;*

*G; E; S ‘ e* : *bool*(*f alse*)*; S₁;* *G; E; S₁ ‘ b₂* : *v; S₂;* [if-else-expr-false] *G; E; S ‘ b₁* if *e* else *b₂* : *v; S₂;*

While Loops. Evaluating a while loop rst evaluates the condition. If the condition is false, the loop terminates. If the condition is true, then the loop body is executed and the while loop is then evaluated again unless the loop body returns a value.

<u>G; E; S ‘ e : bool(f alse); S₁;</u> [while-false] *G; E; S ‘* while *e*: *b* :*; S₁;*

*G; E; S ‘ e* : *bool*(*true*)*; S₁;* *G; E; S₁ ‘ b* :*; S₂;* <u>G; E; S₂ ‘ while e: b :; S₃; R</u> [while-true-loop] *G; E; S ‘* while *e*: *b* :*; S₃; R*

*G; E; S ‘ e* : *bool*(*true*)*; S₁;* *G; E; S₁ ‘ b* :*; S₂; R* *R* is not [while-true-return] *G; E; S ‘* while *e*: *b* :*; S₂; R*

Return Statements. return statements explicitly or implicitly set the return value *R*, which then prop- agates upward, preventing further execution until the nearest enclosing function call is reached.

*G; E; S ‘ e* : *v; S₁;* [return-e] *G; E; S ‘* return *e* :*; S₁; v*

[return] *G; E; S ‘* return :*; S; None*

Statement Sequences. A sequence of statements is evaluated by evaluating each statement in turn, either until some statement returns a value or until the last statement in the sequence is evaluated.

*n* 0 *G; E; S₀ ‘ s₁* :*; S₁;* *G; E; S₁ ‘ s₂* :*; S₂;* ... *G; E; Sn* 1*‘ sn*:*; Sn;* [stmt-seq] *G; E; S₀ ‘ s₁* NEWLINE *s₂* NEWLINE *: : : sn*NEWLINE :*; Sn;*

*n* 0 *G; E; S₀ ‘ s₁* :*; S₁;* *G; E; S₁ ‘ s₂* :*; S₂;* ... *G; E; Sk* 1*‘ sk*:*; Sk; R* *k n; R* is not [stmt-seq-return] *G; E; S₀ ‘ s₁* NEWLINE *s₂* NEWLINE *: : : sn*NEWLINE :*; Sk; R*

Function Invocation and Method Dispatching. These are two of the three most complex operational rules (the other being object creation, [new]). First, simple function invocation.

*0k* *S₀*(*E*(*f*)) = (*x₁; : : :; xn; y₁* = *e⁰*1*; : : :; yk*= *e; bbody; Ef*) *n; k* 0 *G; E; S₀ ‘ e₁* : *v₁; S₁;* ... *G; E; Sn* 1*‘ en*: *vn; Sn;* *lx*1*; : : :; lxn; ly*1*; : : :; lyk*= *newloc*(*Sn; n* + *k*) *E⁰* = *Ef*[*lx*1*=x₁*]*: : :*[*lxn=xn*][*ly*1*=y₁*]*: : :*[*lyk=yk*] *0* *G; E⁰; Sn‘ e⁰*1: *v₁; Sn;* ... *0k 0* *G; E⁰; Sn‘ e* : *vk; Sn;* *0 0* *Sn*+1= *Sn*[*v₁=lx*1]*: : :*[*vn=lxn*][*v₁=ly*1]*: : :*[*vk=lyk*] *G; E⁰; S ‘ bbody*:*; Sn*+2*; R* (*n*+1 *0None;* if *R* is *R* = *R;* otherwise [invoke] *0* *G; E; S₀ ‘ f*(*e₁; : : :; en*) : *R; Sn*+2*;*

First, the function’s value is fetched from the current store. Second, the arguments to the function call are evaluated in left-to-right order. Then, new locations are allocated for the function’s formal parameters, local variables and nested functions. A new environment *E⁰* is created for the function call, which maps the formal parameters, local variables, and the names of nested functions to their corresponding locations. The store *Sn*+1maps these locations to their corresponding arguments, initial values, and function values respectively. Finally, the body of the function is evaluated with this new environment *E⁰* and initial state *Sn*+1. The function invocation expression evaluates to the value returned by the function body, or the value None if the function body was completely evaluated without encountering a return statement.

*G; E; S ‘ e₀* : *v₀; S₀;* *v₀* = *X*(*a₁* = *l₁; : : :; f* = *lf; : : :; am*= *lm*) *0k* *S₀*(*lf*) = (*x₀; x₁; : : :; xn; y₁* = *e⁰*1*; : : :; yk*= *e; bbody; Ef*) *n; k* 0 *G; E; S₀ ‘ e₁* : *v₁; S₁;* ... *G; E; Sn* 1*‘ en*: *vn; Sn;* *lx*1*; : : :; lxn; ly*1*; : : :; lyk*= *newloc*(*Sn; n* + *k*) *E⁰* = *Ef*[*lx*0*=x₀*]*: : :*[*lxn=xn*][*ly*1*=y₁*]*: : :*[*lyk=yk*] *0* *G; E⁰; Sn‘ e⁰*1: *v₁; Sn;* ... *0k 0* *G; E⁰; Sn‘ e* : *vk; Sn;* *0 0* *Sn*+1= *Sn*[*v₀=lx*0]*: : :*[*vn=lxn*][*v₁=ly*1]*: : :*[*vk=lyk*] *G; E⁰; S ‘ bbody*:*; Sn*+2*; R* (*n*+1 *0None;* if *R* is *R* = *R;* otherwise [dispatch] *0* *G; E; S ‘ e₀:f*(*e₁; : : :; en*) : *R; Sn*+2*;*

The rule for dynamic dispatch is similar to the rule for function invocation, with two main dierences: (1) the target method is determined by rst evaluating the object expression and then retrieving the function value that the method slot in the resulting object maps to; (2) the object itself is passed as the rst argument to the method, before any of the arguments in the method-call expression.

Function Denitions. Function denitions result in the creation of function values. The rule is exactly the same for the denition of a globally dened function, a nested function, or a method dened in a class:

*g₁; : : :; gL*are the variables explicitly declared as global in *f* *y₁* = *e₁; : : :; yk*= *ek*are the local variables and nested functions dened in *f* *Ef*= *E*[*G*(*g₁*)*=g₁*]*: : :*[*G*(*gL*)*=gL*] <u>v = (x₁; : : :; xn; y₁ = e₁; : : :; yk= ek; bbody; Ef)</u> [func-method-def] ? *G; E; S ‘* def *f* (*x₁*: *T₁; : : :; xn*: *Tn*) J-> *T₀*K : *b* : *v; S;*

The function value captures the environment *E* in which it is dened. This allows a nested function to refer to local variables and functions dened in enclosing scopes. The captured environment *E* is slightly modied as *Ef*, which overrides the mapping for variables explicitly declared as global within the function’s body using the global declaration. This modication allows a nested function to reference a global variable even if there exists a local variable dened with the same name in an enclosing scope.

Accessing Class Attributes. The rules for accessing class attributes are relatively simple. Here, we use the syntax for class denitions described in Section 6.3 to state the necessary assumptions about the object value being selected from (*v₁*):

*G; E; S₀ ‘ e* : *v₁; S₁;* *v₁* = *X*(*a₁* = *l₁; : : :; id* = *lid; : : :; am*= *lm*) <u>v₂ = S₁(lid)</u> [attr-read] *G; E; S₀ ‘ e:id* : *v₂; S₁;*

*G; E; S₀ ‘ e₂* : *vr; S₁;* *G; E; S₁ ‘ e₁* : *vl; S₂;* *v* *l*= *X*(*a₁* = *l₁; : : :; id* = *lid; : : :; am*= *lm*) <u>S₃ = S₂[vr=lid]</u> [attr-assign-stmt] *G; E; S₀ ‘ e₁:id* = *e₂* :*; S₃;*

In the rule for attribute assignment, the expression on the right-hand side is evaluated before the expression on the left-hand side.

Object Instantiation. Instead of treating classes as values stored in the state, as for functions, the reference chooses to rely on the *class*( ) mapping from Section 6.3.

*class*(*T*) = (*a₁* = *e₁; : : :; am*= *em*) *m* 1 *l* *a*1*; : : :; lam*= *newloc*(*S; m*) *v₀* = *T* (*a₁* = *lai; : : :; am*= *lam*) *G; G; S ‘ e₁* : *v₁; S;* *G; G; S ‘ e₂* : *v₂; S;* ... *G; G; S ‘ em*: *vm; S;* *S₁* = *S*[*v₁=la*1]*: : :*[*vm=lam*] *l* *init*= *lai*such that *ai*= init *S₁*(*linit*) = (*x₀; y₁* = *e⁰*1*; : : :; yk*= *e* *0k* *; bbody; Ef*) *k* 0 *l* *x*0*; ly*1*; : : :; lyk*= *newloc*(*S₁; k* + 1) *E⁰* = *Ef*[*lx*0*=x₀*][*ly*1*=y₁*]*: : :*[*lyk=yk*] *G; E; S₁ ‘ e⁰*1: *v₁* *0* *; S₁;* ... *G; E; S₁ ‘ e* *0k* : *vk* *0* *; S₁;* *S₂* = *S₁*[*v₀=lx*0][*v₁* *0* *=ly*1]*: : :*[*vk* *0* *=lyk*] <u>G; E⁰; S₂ ‘ bbody:; S₃;</u> [new] *G; E; S ‘ T*() : *v₀; S₃;*

This rule performs the following operations. First, a new object *v₀* with class *T* is created by allocating locations for each attribute and method that is dened or inherited by class *T*. Second, the attribute initializers and method denitions are evaluated using the *global environment*; this distinction is important since method denitions do not capture the environment *E* in which the object is being constructed. Third, a new store *S₁* is created by modifying the current store *S* with mappings for the newly allocated attributes and methods of *v₀*. Finally, the init method of the object *v₀* is invoked via dynamic dispatch. The steps required to invoke this method are similar to that of general dynamic dispatch, with the exception that the init method does not accept any arguments apart from the object on which it is invoked.

List Displays. A list display creates a sequence of new locations in the store, which house the values of the list elements.

*n* 0 *G; E; S₀ ‘ e₁* : *v₁; S₁;* *G; E; S₁ ‘ e₂* : *v₂; S₂;* ... *G; E; Sn* 1*‘ en*: *vn; Sn;* *l₁; : : :; ln*= *newloc*(*Sn; n*) *v* = [*l₁; l₂; : : :; ln*] <u>Sn+1= Sn[v₁=l₁][v₂=l₂]: : :[vn=ln]</u> [list-display] *G; E; S₀ ‘* [ *e₁; e₂; : : :; en*] : *v; Sn*+1*;*

Operations on Lists. The rules for list selection, concatenation, and element update all use the locations corresponding to list elements and the values they map to in the store.

*G; E; S₀ ‘ e₁* : *v₁; S₁;* *G; E; S₁ ‘ e₂* : *int*(*i*)*; S₂;* *v₁* = [*l₁; l₂; : : :; ln*] 0 *i < n* <u>v₂ = S₂(li+1)</u> [list-select] *G; E; S₀ ‘ e₁*[*e₂*] : *v₂; S₂;*

*G; E; S₀ ‘ e₁* : *v₁; S₁;* *G; E; S₁ ‘ e₂* : *v₂; S₂;* *v₁* = [*l₁; l₂; : : :; ln*] *v₂* = [*l₁* *0* *; l₂* *0* *; : : :; lm* *0*] *n; m* 0 *l₁* *00* *; : : :; lm* *00* +*n*= *newloc*(*S₂; m* + *n*) *v₃* = [*l₁* *00* *; l₂* *00* *; : : :; ln* *00* +*m*] <u>S₃ = S₂[S₂(l₁)=l₁</u> *00* <u>]: : :[S₂(ln)=ln</u> *00* <u>][S₂(l₁</u> *0* <u>)=ln</u> *00* <u>+1]: : :[S₂(lm</u> *0* <u>)=ln</u> *00* <u>+m]</u> [list-concat] *G; E; S₀ ‘ e₁* + *e₂* : *v₃; S₃;*

*G; E; S₀ ‘ e₃* : *vr; S₁;* *G; E; S₁ ‘ e₁* : *vl; S₂;* *G; E; S₂ ‘ e₂* : *int*(*i*)*; S₃;* *v*

|v|= [l₁; l₂; : : :; l|]|
|---|---|---|
|0|i < n||
|S₄ = S₃[v||]|
|G; E; S₀ ‘ e₁[e₂] = e₃ :||; S₄;|

*l*= [*l₁; l₂; : : :; ln*]

*r* *=li*+1 [list-assign-stmt]

In the last rule, the expression on the right-hand side of the assignment operator is evaluated before the expressions on the left-hand side.

Multiple Assignment. We can describe multiple assignments by a kind of rewriting, breaking them down into a sequence of simple assignments and taking care to evaluate the right-hand side exactly once.

*n >* 1 *l* = *newloc*(*S*) *X*is a unique identier not in the program *G; E; S ‘ e₀* : *v; S⁰;* <u>G; E[l=X]; S⁰[v=l] ‘ e₁ = X NEWLINE e₂ = X NEWLINE : : : en= X :; S⁰⁰;</u> [multi-assign-stmt] *G; E; S ‘ e₁* = *e₂* = = *en*= *e₀* :*; S;*

Predened Functions. As mentioned in Section 6.2.4, the built-in functions have special bodies that do not occur in value ChocoPy programs. The rule for function invocation will give rise to assertions involving these bodies. For those that take an argument, the argument name will be ’val’. The following rules tell what the body does once any function argument has been evaluated and assigned to the variable val. The complete behavior of a call such as len(L) then follows from the rules below plus [invoke]. The predened functions print and input perform IO.

*S*(*E*(val)) = *v* <u>v = int(i) or v = bool(b) or v = str(n; s)</u> [print] *G; E; S ‘* native print :*; S; None*

*s* is the next user-provided input string of length *n* *v* = *str*(*n; s*) [input] *G; E; S ‘* native input :*; S; v*

The predened function len retrieves the length of a list or a string.

*S*(*E*(val)) = *v* *v* = [*l₁; l₂; : : :; ln*] *n* 0 [len-list] *G; E; S ‘* native len :*; S; int*(*n*)

*S*(*E*(val)) = *v* *v* = *str*(*n; s*) [len-str] *G; E; S ‘* native len :*; S; int*(*n*)

For Loops. The rules for for loops on lists and strings update the store after each iteration to map the loop variable’s location to a value corresponding to a list element or substring respectively. In each case, we have a special version for early termination due to a return from the loop body.

*0* *G; E; S₀ ‘ e* : *v; S₀;* *v* = [*l₁; l₂; : : :; ln*] *n* 0 *lid*= *E*(*id*) *0 0* *S₁* = *S₀*[*S₀*(*l₁*)*=lid*] *0* *G; E; S₁ ‘ b* :*; S₁;* *0 0* *S₂* = *S₁*[*S₁*(*l₂*)*=lid*] *0* *G; E; S₂ ‘ b* :*; S₂;* ... *0 0* *Sn*= *Sn* 1[*Sn* 1(*ln*)*=lid*] *0* *G; E; Sn‘ b* :*; Sn;* [for-list] *0* *G; E; S₀ ‘* for *id* in *e* : *b* :*; Sn;*

*G; E; S₀ ‘ e* : *vl; S₀* *0* *;* *v* *l*= [*l₁; l₂; : : :; ln*] *n* 0 *l* *id*= *E*(*id*) *S₁* = *S₀* *0* [*S₀* *0* (*l₁*)*=lid*] *G; E; S₁ ‘ b* :*; S₁* *0* *;* *S₂* = *S₁* *0* [*S₁* *0* (*l₂*)*=lid*] *G; E; S₂ ‘ b* :*; S₂* *0* *;* ... *Sk*= *Sk* *0* 1 [*Sk* *0* 1 (*lk*)*=lid*] *G; E; Sk‘ b* :*; Sk* *0* *; R* 1 *k n R* is not *0* [for-list-return] *G; E; S₀ ‘* for *id* in *e* : *b* :*; Sk; R*

*G; E; S₀ ‘ e* : *v; S₀* *0* *;* *v* = *str*(*n; c₁:c₂: : :cn*) *n* 0 *l* *id*= *E*(*id*) *S₁* = *S₀* *0* [*str*(1*; c₁*)*=lid*] *G; E; S₁ ‘ b* :*; S₁* *0* *;* *S₂* = *S₁* *0* [*str*(1*; c₂*)*=lid*] *G; E; S₂ ‘ b* :*; S₂* *0* *;* ... *Sn*= *Sn* *0* 1 [*str*(1*; cn*)*=lid*] *G; E; Sn‘ b* :*; Sn* *0* *;* *0* [for-str] *G; E; S₀ ‘* for *id* in *e* : *b* :*; Sn;*

*G; E; S₀ ‘ e* : *vs; S₀* *0* *;* *v* *s*= *str*(*n; c₁:c₂: : :cn*) *n* 0 *l* *id*= *E*(*id*) *S₁* = *S₀* *0* [*str*(1*; c₁*)*=lid*] *G; E; S₁ ‘ b* :*; S₁* *0* *;* *S₂* = *S₁* *0* [*str*(1*; c₂*)*=lid*] *G; E; S₂ ‘ b* :*; S₂* *0* *;* ... *Sk*= *Sk* *0* 1 [*str*(1*; ck*)*=lid*] *G; E; Sk‘ b* :*; Sk* *0* *; R* 1 *k n R* is not *0* [for-str-return] *G; E; S₀ ‘* for *id* in *e* : *b* :*; Sk; R*

Programs. Finally, the rule for evaluating a ChocoPy program involves rst initializing a global environment and the initial store with globally dened variables and functions, and then evaluating the sequence of top-level statements. Let*;* represent an empty mapping. Then, the top-level rule is:

*g₁* = *e₁; : : :; gk*= *ek*are the global variable and function denitions in the program *P* is the sequence of statements in the program *lg*1*; : : :; lgk*= *newloc*(*Sinit; k*) *G* = *Ginit*[*lg*1*=g₁*]*: : :*[*lgk=gk*] *G; G; Sinit‘ e₁* : *v₁; Sinit;* ... *G; G;;‘ ek*: *vk; Sinit;* *S* = *Sinit*[*vk=lg*1]*: : :*[*vk=lgk*] <u>G; G; S ‘ P :; S⁰;</u> [program] *0* *;;;;;‘ P* :*; S;*

The initial and store *Ginit*and *Sinit*hold the predened functions:

*Ginit*=*;*[*llen=len*][*lprint=print*][*linput=input*]

#### Sinit(lprint) = (val;native print;;)

#### Sinit(llen) = (val;native len;;)

#### Sinit(linput) = (native print;;)

When no valid rule can be applied to a given expression, the program aborts after printing an appropriate error message. Due to the myriad of semantic checks and typing rules enforced at compile-time, the set of errors that can occur at run-time is limited. The following is the standard set of run-time errors that can occur during the execution of a ChocoPy program:

1.Invalid argument (during invocation of print or len)
2.Division by zero
3.Index out of bounds (during string selection or list element access)
4.Operation on None (during method dispatch, attribute access, or list operations)
5.Out of memory (when allocating a new object) The operational semantics do not specify what happens in the event of arithmetic integer overow. This
manual only species semantics for signed integer arithmetic that ts within 32 bits. Overow is considered undened behavior, and implementations may handle such situations in any manner of their choosing.

## 7 Acknowledgements

ChocoPy is a dialect of Python, version 3.6. The set of Python language features to include in ChocoPy were inuenced by Cool (Classroom Object-Oriented Language), which itself is based on Sather164, a dialect of the Sather language. This language manual is largely based o the Cool reference manual. Several language design choices in ChocoPy were rened through discussions with Rohan Bavishi and Kevin Laeufer. Grant Posner helped review this document and improve its clarity. Countless typos were identied by the students taking CS164 at UC Berkeley in Fall 2018.

## A Known incompatibilities with Python

The following are the known cases where a valid ChocoPy program either does not correspond to a valid Python program, or has dierent semantics:

Python 3 does not allow forward references to classes that have not yet been dened. For example, the variable denition x:A = None is invalid in Python if this line appears before the class A has been completely dened. In ChocoPy, this is valid as long as the class A is dened anywhere in the program. A compromise is to use quoted type annotations: the syntax x:"A" = None is valid in both ChocoPy and Python, and has exactly the same meaning. In Python 3.7, forward references can be enabled by running from future import annotations².

Since integer overow leads to undened behavior in ChocoPy, there is no compatibility with Python when dealing with integer values less than 2 31 or at least as large as 2 31.

2 PEP 563 mentions that forward references will be allowed by default in Python 4.0.