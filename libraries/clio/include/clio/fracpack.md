# ƒracpack ƒormat 

ƒracpack is a zero-copy data serialization format that is a faster alternative
to Google FlatBuffers, Cap'n'Proto. It was created to optimize the performance
of database access and inter-contract communication. It is designed to pack faster
and smaller than FlatBuffers and to work without any code generation in c++ 
(templates and macros aside).

ƒracpack is designed to be forward and backward compatible, allowing old code to
read data packed by newer code and newer code to transparently support data produced
by old code. Data structures can add new optional fields at any time at the end of
the structure but may not reorder or remove old fields. Empty fields are serialized
as all 0's which allows them to be effeciently compressed, if desired, using the
algorithm from Cap'n'Proto.

This document describes the serialized format.

## Basic Types

  The following basic types are serialized directly as if by `memcpy`. 
  
  - char
  - uint8_t
  - uint16_t
  - uint32_t
  - uint64_t
  - int8_t
  - int16_t
  - int32_t
  - int64_t
  - float32
  - float64

## Simple Structs

  Simple structs are any struct which contain only basic types or other simple structs.
  These types are not extensible and are serialized as if using `__attribute__((packed, aligned(1)))`.

  This makes serialization as fast as a memcpy. Simple structs are used as members of extensible structs
  and are never serialized on their own because they would lack size prefix.

## Extensible Structs 

  In addition to fundamental types and simple structs, extensible structs may contain dynamically sized data, 
  such as vectors, strings, optional fields, and other extensible structs.

  The format of an extensibe struct is similar to the following C struct

  ```c++
     struct __attribute__((packed,aligned(1))) extensible {
        uint32_t size;         ///< sizeof(fixed_fields)+sizeof(heap)+1
        uint8_t  fixed_size;   ///< the number of 4-byte words `size` where the heap starts
        char[]   fixed_fields; /// user defined fields, a multiple of 4 bytes in size
        char[]   heap;         /// dynamically allocated data pointed to by user fields
     };

     struct __attribute__((packed,aligned(1))) user_struct {
        extensible header;
        type       userfields...
        char[]     heap;
     }
  ```

## Strings

  Strings are serialized as if they were the following data structure where an
  empty string has a size of 0 and no data or nullterm are included.

  ```c++
  struct string {
     uint32_t     size
     char[size]   data
     char         nullterm;
  }
  ```

## Strings in Structs

  Given the following struct:

  ```c++
  struct strstruct {
    uint16_t      somefield;
    std::string   str = "hello world";
    uint8_t       otherfield;
  };
  ```

  It would be serialized as follows:

  ```c++
  uint32_t size                   = 25 
  uint8_t  fixed_size             = 2 words or 8 bytes   
  ---- start fixed ----
  uint16_t     somefield          =
  uint32_t     offset_ptr         = 6 = sizeof(offsetptr+otherfield+padding)
  uint8_t      otherfield        
  uint8_t      padding            = 0 // fixed_size must be multiple of 4   
  ---- start heap -----
  uint32_t     string::size       = 11;
  char[size]   string::data       = "hello world"
  char         string::nullterm   = "\0";
  ---------------------
  ```

  If the string were null, then there is no heap allocation and the memory of the offset_ptr is
  reinterpreted to be the size of the string and the null terminated string.

  Here is how strstruct would be serialized with an empty string:

  ```c++
  uint32_t size                   = 9 
  uint8_t  fixed_size             = 2 words or 8 bytes   
  ---- start fixed ----
  uint16_t     somefield          =
  uint32_t     offset_ptr         = 0 // string is empty
  uint8_t      otherfield        
  uint8_t      padding            = 0 // fixed_size must be multiple of 4   
  ---- start heap -----
  ---------------------
  ```

## Basic/Simple Vectors

Vectors are always dynamically sized data and utilize the same optimization as empty-strings, so
an empty vector reinterprets the offset_ptr to be the size of the vector.

A vector<Basic|Simple> data is serialized as follows:

```c++
struct vector<T> {
   uint32_t                       number_of_bytes;
   T[number_of_bytes/sizeof(T)]   data;
}
```


## Vectors of Dynamic Data

If a vector contains strings, other vectors, Extensible Structs, optionals, or any other
type that may make use of an offset_ptr then it is serialize as a list of offset pointers
to each pointing to a heap r

```c++
struct vector<T> {
   uint32_t                                number_of_bytes; /// includes size of heap
   uint32_t                                number_of_elements;
   offset_ptr[number_of_elements]          data;
   //---- start heap ----//
   char[]                                  heap;
}
```

## Nested Extensible Structs

If an extensible struct is nested under another struct, then an offset pointer is stored
and the serialized nested member is allocated on the heap. These types cannot be "null"
and there will be no check for a null offset pointer. When validated, pointers to nested
structs must resolve.

## Optional<T> 

Any type can be included as an optional, this means it will be represented as an offsetptr
to an object's heap unless it is an optional type of size less than 3. In that case, the
first byte will indicate the presense of the object and the next 1-3 bytes will store the
data. 

## Optional String/Vector/Variant/Extensible Structs

Optional members that are already an "offset_ptr", such as strings, vectors, and other
extensible types can leverage the existing offset_ptr to encode "not present"; however,
it cannot simply use a `null` offset pointer as string/vector use that to indicate an
empty string or empty vector which is different from a non-present empty vector.

An offset_ptr of value 0,1,2, or 3 points to the same memory that stores the offset_ptr
which means no other object can be stored there. An optional  offset_ptr of 0 means a
string or vector is present but empty string.  An offset_ptr of 1 means not present.

## Struct Extensions
If you want to add new fields to a struct that is already in production, you must add 
them at the end as optionals. If new code is reading data produced by old code, then
the old code will signal a fixed_size less than the new code would otherwise expect. This
means that an "optional" is also "not present" if it's offset from teh start of fixed is
greater than the reported fixed size.  

In practice this means that non-present optionals at the end of an extensible struct have
no on-the wire overhead.


  ```c++
  uint32_t size                   = 25 
  uint16_t fixed_size             = 7
  ---- start fixed ----
  uint16_t     somefield          =
  uint32_t     offset_ptr         = 6 = sizeof(offsetptr+otherfield+padding)
  uint8_t      otherfield        
  optional<string> opt_str;      // a null str = 0, !optional<str>.valid(), then value 1 
  ---- start heap -----
  uint32_t     string::size       = 11;
  char[size]   string::data       = "hello world"
  char         string::nullterm   = "\0";
  int          extra
  ---------------------
  ```

## Variants

  A variant is serialized in the following way:

  ```c++
  uint32_t   size; // the number bytes
  uint8_t    type; // up to 255 types
  char[]     data; 
  ```
