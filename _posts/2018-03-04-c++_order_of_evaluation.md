---
layout:        post
title:         "Order of evaluation of (sub)expressions, the sequenced-before partial order and sequenced-before graphs in c++14/17"
my_category:   "c++"
---
In this post we review the rules concerning the order of evaluation of (sub)expressions in c++14 and c++17.
The sequenced-before partial order binary relation is introduced and copious references to the standard are given.
Sequenced-before graphs are then introduced and used to determine the well-definedness (or undefinedness)
of example expressions.

*note :* << The standard >> means the draft n4659 unless mentioned otherwise. 
  See [this question on stackoverflow][stackoverflow_c++_standard] 


<!-- kramdown links defs -->
[stackoverflow_c++_standard]: https://stackoverflow.com/questions/81656/where-do-i-find-the-current-c-or-c-standard-documents 

