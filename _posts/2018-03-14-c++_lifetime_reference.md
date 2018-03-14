---
layout:         post
title:          "Extension of the lifetime of temporaries by binding to a reference: general rule and three exceptions"
my_category:    "c++"
---
This is a quick note on the rules concerning the extension the lifetime of temporaries when bound to a reference.
The main rule is introduced, the three exceptions presented and an example is given.
<!--more-->

The << standard >> means the draft [n4659][draft_n4659] unless mentioned otherwise.
This draft should be very close to the final c++17 standard.
(See [this question on stackoverflow][stackoverflow_c++_standard])

### The rules

The rules are given in 15.2 [class.temporary]/6.

In general when a temporary is bound to a reference its lifetime
is extended to the lifetime of the reference, with three exceptions:
 1. *ref. param* --- When a temporary is bound to a reference parameter of a function call,
   the lifetime of the temporary is extended until the end of the full expression.
 2. *return value* --- The lifetime of a temporary bound to the return value of a function is *not*
   extended.
 3. *new expression* --- When a temporary is bound to a reference in a new expression,
   the lifetime of the temporary is extended until the end of the full expression
   (and not until the delete).

### Example
Given:

~~~ c++ {% raw %}
#include <iostream>
#define PRETTY(x) (std::cout << __PRETTY_FUNCTION__ << " : " << (x) << std::endl)

struct X{
    X() { PRETTY(this); }
    ~X() { PRETTY(this); }
};

struct Y {
    int i;
    const X& rx;
}; {% endraw %}
~~~

#### Example: general case
In general the lifetime of temporaries is extended:

~~~ c++ {% raw %}
const X& rx = X{};
std::cout << "Do stuff with rx... " << &rx << '\n'; {% endraw %}
~~~

will print:

~~~ {% raw %}
X::X() : 0x7ffd8fa6a338
Do stuff with rx... 0x7ffd8fa6a338
X::~X() : 0x7ffd8fa6a338 {% endraw %}
~~~
#### Example: exception << *ref. param* >>

~~~ c++ {% raw %}
auto lambda = [](const X& rx) -> void { std::cout << "Do stuff with rx... " << &rx << '\n'; };
(lambda(X{}), std::cout << "Do stuff after calling lambda" << '\n'); {% endraw %}
~~~

will print:

~~~ {% raw %}
X::X() : 0x7ffd8fa6a320
Do stuff with rx... 0x7ffd8fa6a320
Do stuff after calling lambda
X::~X() : 0x7ffd8fa6a320 {% endraw %}
~~~

[draft_n4659]: {{ site.baseurl }}{% link /assets/c++_order_of_evaluation/n4659_final_c++17.pdf %}
[stackoverflow_c++_standard]: https://stackoverflow.com/questions/81656/where-do-i-find-the-current-c-or-c-standard-documents
