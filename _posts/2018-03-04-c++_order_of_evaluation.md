---
layout:        post
title:         "Order of evaluation of (sub)expressions, the sequenced-before partial order and sequenced-before graphs in c++14/17"
my_category:   "c++"
---
{% comment %}
In this post we review the rules concerning the order of evaluation of (sub)expressions in c++14 and c++17.
The sequenced-before partial order binary relation is introduced and copious references to the standard are given.
Sequenced-before graphs are then introduced and used to determine the well-definedness (or undefinedness)
of example expressions.

*The standard* means the draft [n4659][draft_n4659] unless mentioned otherwise. 
This draft should be very close to the final c++17 standard.
(See [this question on stackoverflow][stackoverflow_c++_standard])
We will indicate which rules apply only to c++17 and which rules apply to both
c++17 and c++14.

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
   An evaluation of a (sub)expression involes in general both a value computation
   (the result of the evaluation of the expression) and a side effect computation
   (the side effects of the evaluation of the expression).
   We will abbreviate the value computation by *val* and the side effect computation
   by *se*. (4.6 [intro.execution]/14)

 - *partial order* ---
   A partial order $$\leq$$ on a set $$S$$ is a binary relation on $$S$$ that is:
   - transitive: $$ \forall a,b,c \in S \text{ , } a \leq b \text{ and } b \leq c \Rightarrow a \leq c $$
   - antisymmetric: $$ \forall a,b \in S \text{ , } a \leq b \text{ and } b \leq a \Rightarrow a = b$$
   - reflexive: $$ \forall a \in S \text{ , } a \leq a $$

 - *strict partial order* ---
   A strict partial order $$ < $$ on a set $$S$$ is a binary relation on $$S$$ that is:
   - transitive (see above)
   - irreflexive: $$ \nexists a \in S \text{ such that } a < a $$
   
   A partial order $$\leq$$ on a set $$S$$ is equivalent to a strict partial order $$ < $$ on $$S$$
   by defining $$ a < b \Leftrightarrow a \leq b \text{ and } a \neq b $$

 - *the sequenced-before strict partial order* ---
   The standard defines (4.6 [intro.execution]/15) the sequenced-before strict partial order on
   the set of value/side effect computations *in a single thread* by $$ a < b \Leftrightarrow a \text{ occurs before } b $$,
   where $$a$$ and $$b$$ are two value/side effect computations. As a notation simplification we will write $$E_1 < E_2$$
   to mean << all value and side effect computations, recursively, of $$E_1$$ are sequenced before
   all value and side effect computations, recursively, of $$E_2$$ >>.

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

{% endcomment %}
