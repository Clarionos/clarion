#pragma once  
namespace clio {
namespace {
  inline constexpr uint64_t unaligned_load(const char* p)
  {
    uint64_t r = 0;
    for( uint32_t i = 0; i < 8; ++i ) {
       r |= p[i];
       r <<= 8;
    }
    return r;
  }

  // Loads n bytes, where 1 <= n < 8.
  inline constexpr uint64_t load_bytes(const char* p, int n)
  {
    uint64_t result = 0;
    --n;
    do
      result = (result << 8) +  (unsigned char)(p[n]);
    while (--n >= 0);
    return result;
  }

  inline constexpr uint64_t shift_mix(std::uint64_t v)
  { return v ^ (v >> 47);}
} // namespace

// Implementation of Murmur hash for 64-bit size_t.
  inline constexpr uint64_t murmur64(const char* ptr, uint64_t len, uint64_t seed = 0xbadd00d00)
  {
    const uint64_t mul = (((uint64_t) 0xc6a4a793UL) << 32UL)
			      + (uint64_t) 0x5bd1e995UL;
    const char* const buf = ptr;

    // Remove the bytes not divisible by the sizeof(uint64_t).  This
    // allows the main loop to process the data as 64-bit integers.
    const uint64_t len_aligned = len & ~(uint64_t)0x7;
    const char* const end = buf + len_aligned;
    uint64_t hash = seed ^ (len * mul);
    for (const char* p = buf; p != end; p += 8) {
     const uint64_t data = shift_mix(unaligned_load(p) * mul) * mul;
     hash ^= data;
     hash *= mul;
    }
    if ((len & 0x7) != 0) {
     const uint64_t data = load_bytes(end, len & 0x7);
     hash ^= data;
     hash *= mul;
    }
    hash = shift_mix(hash) * mul;
    hash = shift_mix(hash);
    return hash;
  }
} // namespace eosio
