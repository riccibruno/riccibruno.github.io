---
layout:        post
title:         "Order of evaluation of (sub)expressions, the sequenced-before partial order and sequenced-before graphs in c++14/17"
my_category:   "c++"
---
**⌛in construction ⌛**

In this post we review the rules concerning the order of evaluation of (sub)expressions in c++14 and c++17.
The sequenced-before partial order binary relation is introduced and copious references to the standard are given.
Sequenced-before graphs are then introduced and used to determine the well-definedness (or undefinedness)
of example expressions.

*The standard* means the draft [n4659][draft_n4659] unless mentioned otherwise. 
This draft should be very close to the final c++17 standard.
(See [this question on stackoverflow][stackoverflow_c++_standard])
We will indicate which rules apply only to c++17 and which rules apply to both
c++17 and c++14. For the precise wording of the rules and definitions please consult the standard.

We first need some definitions. Skip them if you are already familiar with them:

 - *observable behaviours* ---
   Only the observable behaviours are required to be emulated by a conforming
   implementation. This rule is sometimes called the *as-if rule* since
   the implementation can do whatever it wants as long as from the point of view
   of the observable behaviours the program behaves as written. (4.6 [intro.execution]/1)
   The observable behaviours are (4.6 [intro.exection]/7):
    - accesses to volatile glvalues
    - data written to files at program termination
    - during input/output, prompting output must be delivered before waiting for input

 - *side effects* ---
   The following are defined to be side effects (4.6 [intro.execution]/14):
    - reading a volatile glvalue
    - modifying an object
    - calling a library I/O function
    - calling a function which does any of the above

 - *value/side effects computation* ---
   An evaluation of an expression involes in general both a value computation
   (the result of the evaluation of the expression) and a side effect computation
   (the side effects of the evaluation of the expression) (4.6 [intro.execution]/14).
   Given an expression $$e$$, we will abbreviate the value computation of $$e$$ by val $$e$$
   and the side effect computation of $$e$$ by se $$e$$.

 - *partial order* ---
   A partial order $$\leq$$ on a set $$S$$ is a binary relation on $$S$$ that is:
   - transitive: $$\forall a,b,c \in S$$, $$a \leq b$$ and $$b \leq c \Rightarrow a \leq c$$
   - antisymmetric: $$\forall a,b \in S$$, $$a \leq b$$ and $$b \leq a \Rightarrow a = b$$
   - reflexive: $$\forall a \in S$$, $$a \leq a $$

 - *strict partial order* ---
   A strict partial order $$ < $$ on a set $$S$$ is a binary relation on $$S$$ that is:
   - transitive (see above)
   - irreflexive: $$ \nexists a \in S$$ such that $$a < a$$
   
   A partial order $$\leq$$ on a set $$S$$ is equivalent to a strict partial order $$ < $$ on $$S$$
   by defining $$a < b \Leftrightarrow a \leq b$$ and $$a \neq b$$.

 - *the sequenced-before strict partial order* ---
   The standard defines (4.6 [intro.execution]/15) the sequenced-before strict partial order on
   the set of value/side effect computations *in a single thread* by $$a < b \Leftrightarrow a$$ occurs before $$b$$,
   where $$a$$ and $$b$$ are two value/side effect computations. As a notation simplification,
   given two expressions $$e_1$$ and $$e_2$$  we will write $$e_1 < e_2$$
   and say that $$e_1$$ is sequenced before $$e_2$$
   to mean << all value and side effect computations, recursively, of $$e_1$$ are sequenced before
   all value and side effect computations, recursively, of $$e_2$$ >>.

 - *well-defined/implementation-defined/unspecified/undefined behaviors* ---
   Behaviors can fall into four categories defined in (4.6 [intro.execution]/1-5):
   - *well-defined* ---
     The behavior is defined by the standard.
   - *implementation-defined* ---
     The standard specifies a range of possible behaviors and the
     implementation must document what is the actual behavior.
   - *unspecified* ---
     The standard specifies a range of possible behaviors but
     the actual behavior in this range is not specified and might
     vary from one execution of a program to another.
   - *undefined* ---
     The standard imposes absolutely no requirements on
     the behavior of programs which contains undefined behaviors.

 - *memory location* ---
   A memory location is one of the following (4.4 [intro.memory]/3):
   - object of scalar type (which are possibliy cv-qualified arithmetic types, enumeration types, pointer types,
     pointer-to-member types and std::nullptr_t (6.9 [basic.types]/9))
   - maximal sequence of adjacent bitfields all having non-zero width

 - *unsequenced/indeterminately sequenced* ---
   Given two value/side effect computation $$a$$ and $$b$$, $$a$$ and $$b$$ are said to
   be unsequenced if neither $$a < b$$ nor $$b < a$$. $$a$$ and $$b$$ are said to be
   indeterminately sequenced if either $$a < b$$ or $$b < a$$, but which one is unspecified
   (4.6 [intro.execution]/15).

We can now give the criterion for the well-definedness of an expression.
The behavior is well-defined unless one of the following is true, in
which case the behavior is undefined (4.6 [intro.execution]/17):
 - A side effect on a memory location is unsequenced with another side effect
   on the same memory location
 - A side effect on a memory location is unsequenced with a value computation
   on the same memory location, where the value computation uses the value in the
   memory location

Unless otherwise specified evaluations are unsequenced.
The sequencing rules can be classified into 3 categories
(in the standard the rules are given all over the place, search for
<< sequenced before >>):
 - *basic rules*
 - *specific rules*
 - *rules added in c++17*

The basic rules are:
 1. Given a full expression $$e_1$$ and the next full expression $$e_2 \Rightarrow e_1 < e_2$$.
 2. Given an operator @ and operands $$e_i$$, val $$e_i < $$ val @.
   (note that nothing is said about the side effects)
 3. In a function call postfix-expression(arg expressions), every argument expressions and
   the postfix-expression designating the call are sequenced before every expressions in the function body.
   Additionally, function calls are indeterminately sequenced if not otherwise sequenced.

The specific rules are (where @ is a placeholder for one of the appropriate operators):
 1. Postfix \+\+/\-\- $$\Rightarrow$$ val @ < se @.
 2. Prefix \+\+/\-\- with operand $$e \Rightarrow$$ val $$e < $$ se @ $$< $$ val @.
 3. Logical && and \|\| with operands $$e_1$$ and $$e_2$$ ($$e_1$$ @ $$e_2$$) $$\Rightarrow e_1 < e_2$$.
 4. Ternary ?: with operands $$e_1$$, $$e_2$$ and $$e_3$$ ($$e_1$$ ? $$e_2$$ : $$e_3$$) $$\Rightarrow e_1 < e_2$$ and $$e_1 < e_3$$.
 5. Comma , with operands $$e_1$$ and $$e_2$$ ($$e_1$$, $$e_2$$) $$\Rightarrow e_1 < e_2$$.
 6. (Compound) assignment @ $$\in \{$$ =,\+=,\-=,\*=,/=,%=,^=,\|=,&=,\<\<=,\>\>= $$\}$$ \\
   with operands $$e_1$$ and $$e_2$$ ($$e_1$$ @ $$e_2$$) $$\Rightarrow$$ val $$e_1$$ and val $$e_2 < $$ se @ $$< $$ val @.
 7. Braced-init-list \{ $$e_1$$, $$e_2$$, ..., $$e_n$$  \} $$\Rightarrow$$ $$\forall i=1,\cdots,n-1$$ we have $$e_i < e_{i+1}$$.
   {% comment %}_ this is because vim is confused by the \_{% endcomment %}
 8. Return statement $$\Rightarrow$$ copy init of the result $$< $$ destruction of temporaries at the end of the full expression
   $$< $$ destruction of the local variables in the block.
 9. New expression $$\Rightarrow$$ the allocation function is indeterminately sequenced with the expression in the initializer,
   and the initialization of the allocated object $$< $$ value computation of the new expression.

The rules added in c++17 are:
 1. Subscript operator [] with operands $$e_1$$ and $$e_2$$ ($$e_1$$[$$e_2$$]) $$\Rightarrow e_1 < e_2$$.
 2. Pointer to member operator .\* / pointer to member of pointer operator ->\* with operands $$e_1$$ and $$e_2$$\\
   ($$e_1$$@$$e_2$$) $$\Rightarrow e_1 < e_2$$.
 3. Arithmetic shifts \<\</\>\> with operands $$e_1$$ and $$e_2$$ ($$e_1$$ @ $$e_2$$) $$\Rightarrow e_1 < e_2$$.
 4. (Compound) assignment (see above rule 6.) $$e_1$$ @ $$e_2 \Rightarrow e_2 < e_1$$.
 5. Function call postfix-expression(arg expressions) $$\Rightarrow$$
    - Postfix-expression $$< $$ $$arg expressions$$ and $$default argument expressions$$.
    - Argument expressions are indeterminately sequenced instead of being unsequenced.
    - For an operator invoked using the operator notation, the operands are sequenced as
      for built-in operators.
 6. New expression $$\Rightarrow$$ the allocation function is sequenced before the expressions in the initializer.
 7. Parenthesized initializer ($$e_1$$,...$$e_n) $$\Rightarrow$$ $$\forall i=1,\cdots,n-1$$ we have $$e_i < e_{i+1}$$.
   {% comment %}_ this is because vim is confused by the \_{% endcomment %}

<!-- kramdown links defs -->
[draft_n4659]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/n4659_final_c++17.pdf %}
[stackoverflow_c++_standard]: https://stackoverflow.com/questions/81656/where-do-i-find-the-current-c-or-c-standard-documents
[defs]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/defs.png %}
[example1]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/example1.png %}
[example2]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/example2.png %}
[example3]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/example3.png %}
[example4]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/example4.png %}
[example5]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/example5.png %}
[example6]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/example6.png %}
[example7]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/example7.png %}
[example8]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/example8.png %}
[example9]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/example9.png %}
[example10]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/example10.png %}
