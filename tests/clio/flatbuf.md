# CLIO Flat Buffer Format

CLIO Flat Buffers provide a way to serialize data that can be directly accessed without
requiring an unpack step. In many cases accessing the packed type is faster than accessing
the rich C++ type due to better memory locality. 

## How does this compare to Google Flatbuffers?
1. Google FB has all aligned types, which CLIO should probably adopt
2. Google FB uses Structs just like CLIO, but without dynamic sized members
3. Google FB uses Tables for anything that CLIO deems dynamic struct
   - tables start with offset to a vtable
   - tables may be stored anywhere relative to an object, so +/- offsets are possible
   - This vtable offset is followed by all fields as offsets, unlike structs, not
     all fields need to be present and there is no set order or layout. A table may
     contain field offsets that point to the same value (cheap de-dup)
   - vtables are shared between objects taht have the same values
   - the elements in the vtable are offset (16bit) the first is size of vtable, the
     second is the size of the object (number of fields).

Some relevant notes about Google's approach:
1. reading a value from a table requires 
       a. offset to vtable
       b. various checks for optionally present data
       c. offset to actual data
       d. extra data for vtable 


The vtable approach is designed to allow returning of "default" values when not present.
The apporach is good for forward/backward compatibility and evolving protocols. That said 
you always have to include all fields in the vtable even if all fields don't appear in the data.

The usecase for CLIO in blockchains which don't have a lot of "optional" fields and in smart 
contracts which have stable data makes the whole vtable thing more complex. 

The hardest part about serializing Google FB is maintaining the "global" set of vtables based on
the types involved... I suppose a multi-pass reflection over the whole type, first extracting all
of the vtables, then serializing things... the vtable is entirely unnecessary if all fields are
always required.  Furthermore, GFB doesn't allow optimization in case of hybird "tables" where some fields
can be inline others offset. 


## Arithematic Types (int, char, double, etc)
These types are serialized directly. 


## Trivially Copyable Types
For trivially copyable types that are not reflected, 
the format is identical to the C representation and the buffer 
can simply be reinterpret cast. Any padding is presumed to be 0.

## Reflected Trivial Copyable Types
These types are packed as if using the `__attribute__ ((packed))` 
where the fields are orded as listed by the macro (not in the struct definition)

Consider the following `data` struct

```c++
{
  struct data {
     uint64_t c;
     uint16_t a;
     uint32_t b;
  };
  CLIO_REFLECT( data, (c)(b)(a) )
}
```

It would be packed as if it were the following struct
```c++
{
  struct __attribute__(__packed__) data {
     uint64_t c;
     uint32_t b;
     uint16_t a;
  };
}
```

## Nested Trival Types

Given a struct containing trivial types, it is packed just like C++ would via a memcpy.

```c++
{
  struct nested {
     int a;
     int b;
  }

  struct data {
     uint64_t c;
     uint32_t b;
     uint16_t a;
     nested   nest;
  };
}
```


## Vectors of Trival Types

A vector of trivial types is size prefixed. 

```
uint32_t size
type[0]
type[1]
type[2]
...
type[size-1]
```

## Strings

A string is packed like a vector<char> and contains both a size and a `null` terminator. This allows
c_str() to work without having to copy it. 


## Structs with Dynamic Members 

If a struct is not trivially copyable, it probably has dynamic members such as strings and vectors.

Consider:

```c++
struct dynamic_member {
   string           str = "hello";
   vector<uint32_t> vec = {1, 2};
};
```

In this case, the dynamically sized members become offset pointers 
which point to a buffer starting after vec. Offset pointers are unsigned which means they always
point forward in the buffer.

```
0] uint32_t str_offset = 8;
4] uint32_t vec_offset = 14 = 4 + (string pack size = 4+5+1=10);
8] uint32_t str_size   = 5; // doesn't include implied nullterm
12] char[6] "hello\0"
18] uint32_t vec_size = 2;
22] uint32_t vec[0] = 1
26] uint32_t vec[1] = 2
```

There is an optimization for strings and vectors of size 0. In that case, the offsetptr is set to 0.

```c++
struct dynamic_member {
   string           str = "";
   vector<uint32_t> vec = {};
};
```

```
0] uint32_t str_offset = 0;
4] uint32_t vec_offset = 0;
```

This works because a 32 bit offsetptr = 0 which is identical to a 
size of 0 which is also identical to a null terminator. This means the
data can be stored in place.


## Structs containing Dynaimc Structs

Consider a struct that has a dynamic struct as a member:

```c++
struct dynamic_struct {
   string           str = "";
   vector<uint32_t> vec = {};
};

struct dynamic_member {
   int64_t        data;
   dynamic_struct member;
   int32_t        other;
};
```

In this case the dynamic member is stored as an offset ptr which points to 
a length-prefixed data blob which contains the serialized form of member.

```
0]  uint64_t data; /// just for fun
8]  uint32_t offsetptr = 8; /// comes after this and other
12] uint32_t other
16] uint32_t size_of_member = 8
20] uint32_t str_offset = 0;
24] uint32_t vec_offset = 0;
```

## Structs with Optional Members

Like empty strings and vectors, a null offset ptr can represent an empty optional, if the
optional struct is fixed size, then it will point to a buffer without a size prefix, otherwise, 
it will point to a buffer with a size prefix.

```c++
struct dynamic_member {
   int64_t                   data;
   optional<struct>          member;
   optional<dynamic_struct>  member;
   int32_t                   other;
};
```

## Vectors of Dynamic Types

Vectors of dynamic types turn into vectors of offset pointers.


## Variants

Packing something like `std::variant<int64_t,dynamic_type,uint8_t>` has the following data:

```
uint64_t   type;  /// globally unique identifier
uint32_t   data;  /// padding
offset_ptr offset_data; /// offset for types that are larger than 64bit.
```

If the c++ name can be base32 encoded in 64bits then that will be the type, otherwise, the
type is the murmur64 hash of the typename as defined by CLIO_REFLECT.

If the content of the variant is a static size less than 8 bytes, then it can be found at &data,
otherwise it is found at &offset_data + offset_data.


## Tuples

Tuples are packed as if the types were included in packed c++ struct.
















Dynamicly Sized Types

Anything that isn't trival requires dynamic memory allocation and is therefore more
difficult to pack. CLIO uses offset pointers to a dynamically allocated memory region 
at the end of the object.

Consider `std::vector<uint64_t>`, 









