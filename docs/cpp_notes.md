# In C++ const/non-const reference makes function overloaded; while pass-by-value does not care about const-ness.

  ```
  void func(int& a);        // takes a non-const reference
  void func(const int& a);  // takes a const reference
  ```
  Above are two different function prototype.

  ```
  void func(int a)
  void func(const int a)
  ```

  Above are two same function prototype.

# 'static' keyword in C++ class (storage modifier)

  1. A static member (variable or function) belongs to the class itself, not to any individual object.
  2. To access static keyword, use "class_name::static_variable" instead of instance.
  3. For classes that contains static methods only, the constructor and destructor could be hidden as 'private' and mark 'delete'.
  4. I personally prefer keeping declaration and definition of static variable separate. In header, define variable as 'static', this does not allocate memory; in source, "type class_name::static_variable = init_val" allocates memory and initializes the value.
  5. It is also possible to allocate memory directly in header using 'constexpr', yet I prefer to have definition separate from interface.
  6. 'static' is good for implementing singleton design pattern, and avoids declaring global instance.
  7. 'static' members are not inherited, they belong to base class itself, and should accessed "base_class_name::static_variable". Thus, there's no polymorphism at all.

# 'static' keyword outside of C++ class
  1. Linkage scope specifier: Used to define function / memory within the scope of current file, like how it is used in C.
  2. Storage modifier: Used to allocate memory in .BSS or .DATA segment depending on whether it is initialized. In other word, not on STACK.

# You cannot use 'const' keyword at end of a 'static' member function
  1. 'static' member function do not have 'this' pointer which is specific for instance.
  2. 'const' qualifier is only tied to instance member variables.

# explicit instantiation of template
  1. Templates are instantiated at compile time, and the compiler needs the full template definition to generate code for whatever type you're using.
  2. Separating template declaration and definition like normal class will cause linker stage.
  3. As solution, EITHER provide definition with declaration in header, OR use .ipp (inline implementation file) for pre-processor to include, OR use explicit instantiation.

# explicit instantiation （模版显式实例化） vs. explicit specialization （模版显式特化） of template
  1. explicit instantiation: Generate code for specific type mentioned explicitly, but using same logic.
  2. explicit specialization: Having completely new logic for a type that's different from original template implementation.

# overloading '[]' operator
  1. Example of overloading '[]' operator in linkedlist.hpp:
  ```
  list_node_t<T>* operator[](size_t& index) const;
  ```
  2. CPP reference: https://en.cppreference.com/w/cpp/language/operators
  3. The operator[] can only be overloaded as a non-static member function, similar to '()', and '->'. It cannot be a non-member or a static member function.
  4. Because a[b] is basically
  ```
  a.operator[](b);
  ```

# overloading '<<' operator
  1. Example of overloading '<<' operator in thread.hpp.
  2. It is quite difference from '[]'. Because 'std::cout << obj;', std::cout is an ostream object, not access by class you have defined. So operator<< must have std::ostream& on the left-hand side. That means you can't make it a member of your class, because your class isn’t the left-hand operand.
  3. Correct typical way to overload '<<':
  ```
    // friend declaration gives access to private members if needed
    friend std::ostream& operator<<(std::ostream& os, const MyClass& obj);
  ```

# 'friend' keyword
  1. A 'friend' function declared in class is a regular non-member function, but has access to its members (including private).
  2. It is not inherited, because it is NOT part of the class.
  3. It is useful in case such as 1. unit test 2. overloading '<<' operator when you expose internal variables to other object.
  4. It’s not called with a 'this' pointer like member functions.
  5. It does not affect size / layout of the class object.
  6. When you declare a 'friend' function to the class, you cannot specifier storage qualifier such as 'static' in that declaration.
  7. 'friend' keyword is only used in declaration of a function in class, it should not be in actual function definition. "Friendship is declared, not defined."

# how class/struct member function works
  1. None 'static' or 'friend' functions are normal functions with an extra argument in the front, 'this', which is pointer pointing to current class instance.
  2. 'struct', like 'class' in C++, also have a 'this' pointer.

# 'placement' 'new'
  ```
  void* buffer = std::malloc(sizeof(MyClass)); // request a raw memory
  MyClass* obj = new(buffer) MyClass(42);      // 'placement new' to construct (call constructor) of MyClass in the pre-allocated memory
  ```

  Below is used in mempool implementation: 'placement new' and 'variadic template'

  ```
  template<typename T, typename... variadic_args_type>
  T* mempool::malloc_construct(variadic_args_type&&... variadic_arg) {
    mempool::alloc_t alloc_res = mempool::malloc(sizeof(T));  // malloc size of type T
    if (alloc_res.status == ALLOC_FAIL) return nullptr;       // nothing special, just status check
    return new (alloc_res.addr) T(variadic_arg...);           // use 'placement new' syntax to call constructor of class type 'T' at the location specified by alloc_res.addr, expanding variadic args into the parameter list of T's constructor.
  }
  ```

  1. the ellipsis (...) operator is used in functions to represent a variadic argument list, allowing a function to accept a variable number of arguments.

# 'template class' versus 'template function'
  1. A 'template function' is a function that is parameterized by type. It lives outside any class.
  2. A 'template class' - entire class is parameterized. Functions inside the class can use the class's template parameter.
  3. 'template function' in regular class- a member function of a regular class can itself be a template.

# Value dereference on linker script defined symbols
  1. When certain symbols are defined in linker script, such as '_ld_start_heap' (specifying starting address of the heap), it is important to specify the type in CPP correctly.
  2. For example, when specifying as
  ```
  extern size_t _ld_start_heap;
  ```

  or

  ```
  extern void* _ld_start_heap;
  ```
  The linker automatically dereferences from this address, meaning take the value on this address and assign to _ld_start_heap in cpp context. It is not expected behavior as the address is expected to be stored as size_t only.

  As fix, it is necessary to declare it correctly as
  ```
  extern void* _ld_start_heap[];
  ```

  and typecast as size_t

  ```
    mempool::heap_start = reinterpret_cast<size_t>(_ld_start_heap);
  ```


