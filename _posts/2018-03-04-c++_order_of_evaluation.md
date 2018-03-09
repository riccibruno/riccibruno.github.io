---
layout:        post
title:         "Order of evaluation of (sub)expressions, the sequenced-before partial order and sequenced-before graphs in c++14/17"
my_category:   "c++"
---
In this post we review the rules concerning the order of evaluation of (sub)expressions in c++14 and c++17.
The sequenced-before partial order binary relation is introduced and copious references to the standard are given.
Sequenced-before graphs are then introduced and used to determine the well-definedness (or undefinedness)
of example expressions.

<< The standard >> means the draft [n4659][draft_n4659] unless mentioned otherwise. 
This draft should be very close to the final c++17 standard.
(See [this question on stackoverflow][stackoverflow_c++_standard])
We will indicate which rules apply only to c++17 and which rules apply to both
c++17 and c++14.

<!-- html comment -->

We first need some definitions. Skip them if you are already familiar with them:

 - *observable behaviours*
   Only the observable behaviours are required to be emulated by a conforming
   implementation. This rule is sometimes called the *as-if rule* since
   the implementation can do whatever it wants as long as from the point of view
   of the observable behaviours the program behaves as written. (4.6 [intro.execution]/1)
   The observable behaviours are (4.6 [intro.exection]/7):
    - accesses to volatile glvalues
    - data written to files at program termination
    - during input/output, prompting output must be delivered before waiting for input

 - *side effects*
   The following are defined to be side effects (4.6 [intro.execution]/14):
    - reading a volatile glvalue
    - modifying an object
    - calling a library I/O function
    - calling a function which does any of the above

<!-- kramdown links defs -->
[draft_n4659]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/n4659_final_c++17.pdf %}
[stackoverflow_c++_standard]: https://stackoverflow.com/questions/81656/where-do-i-find-the-current-c-or-c-standard-documents
