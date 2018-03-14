#include <iostream>
#define PRETTY(x) (std::cout << __PRETTY_FUNCTION__ << " : " << (x) << std::endl)

struct X{
    X() { PRETTY(this); }
    ~X() { PRETTY(this); }
};

struct Y {
    int i;
    const X& rx;
};

int main(){
    /* temporaries lifetimes and references */
    std::cout << "**********" << '\n';
{   
    /* in general the lifetime of the temporary is extended */
    const X& rx = X{};
    std::cout << "Do stuff with rx... " << &rx << '\n';
}
    std::cout << "**********" << '\n';
{
    /* exception 1 : when bound to reference arg of a function call,
     * the lifetime of the temporary is extended until the end of the 
     * full expression. */

    auto lambda = [](const X& rx) -> void { std::cout << "Do stuff with rx... " << &rx << '\n'; };
    (lambda(X{}), std::cout << "Do stuff after calling lambda" << '\n');
}
    std::cout << "**********" << '\n';
{
    /* exception 2 : the lifetime of a temporary bound to the return value of a function is not extended;
     * the temporary is destroyed immediately after the return statement. */ 

    auto lambda = []() -> X& { X x{}; std::cout << "Do stuff inside the lambda with x... " << &x << '\n'; return x;};
    X& rx = lambda();
    std::cout << "Do stuff after calling lambda with rx, oops rx is dangling... " << &rx << '\n';
}
    std::cout << "**********" << '\n';
{
    /* exception 3 : when bound to a ref in a new-expression, the lifetime of a temporary
     * is extended until the end of the full expression */
    Y* py = nullptr;
    (py = new Y{42, X{}}, std::cout << "Do stuff with py->rx..." << &(py->rx) << '\n');
    std::cout << "Do more stuff with py->rx, oops dangling reference... " << &(py->rx) << '\n';
    delete py;
}
    std::cout << "**********" << '\n';
    return 0;
}
