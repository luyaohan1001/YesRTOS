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

# 'static keyword' in C++ class

  1. A static member (variable or function) belongs to the class itself, not to any individual object.
  2. To access static keyword, use "class_name::static_variable" instead of instance.
  3. For classes that contains static methods only, the constructor and destructor could be hidden as 'private' and mark 'delete'.
  4. I personally prefer keeping declaration and definition of static variable separate. In header, define variable as 'static', this does not allocate memory; in source, "type class_name::static_variable = init_val" allocates memory and initializes the value.
  5. It is also possible to allocate memory directly in header using 'constexpr', yet I prefer to have definition separate from interface.
  6. 'static' is good for implementing singleton design pattern, and avoids declaring global instance.
  7. 'static' members are not inherited, they belong to base class itself, and should accessed "base_class_name::static_variable". Thus, there's no polymorphism at all.


# explicit instantiation of template
  1. Templates are instantiated at compile time, and the compiler needs the full template definition to generate code for whatever type you're using.
  2. Separating template declaration and definition like normal class will cause linker stage.
  3. As solution, EITHER provide definition with declaration in header, OR use .ipp (inline implementation file) for pre-processor to include, OR use explicit instantiation.

# explicit instantiation （模版显式实例化） vs. explicit specialization （模版显式特化） of template
  1. explicit instantiation: Generate code for specific type mentioned explicitly, but using same logic.
  2. explicit specialization: Having completely new logic for a type that's different from original template implementation.


