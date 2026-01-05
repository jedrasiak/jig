# RFC 9562

## 4.1. Variant Field
The variant field determines the layout of the UUID. That is, the interpretation of all other bits in the UUID depends on the setting of the bits in the variant field. As such, it could more accurately be called a "type" field; we retain the original term for compatibility. The variant field consists of a variable number of the most significant bits of octet 8 of the UUID.

Table 1 lists the contents of the variant field, where the letter "x" indicates a "don't-care" value.

Table 1: UUID Variants
```
MSB0	MSB1	MSB2	MSB3	Variant	Description
0	x	x	x	1-7	Reserved. Network Computing System (NCS) backward compatibility, and includes Nil UUID as per Section 5.9.
1	0	x	x	8-9,A-B	The variant specified in this document.
1	1	0	x	C-D	Reserved. Microsoft Corporation backward compatibility.
1	1	1	x	E-F	Reserved for future definition and includes Max UUID as per Section 5.10.
```

Interoperability, in any form, with variants other than the one defined here is not guaranteed but is not likely to be an issue in practice.

Specifically for UUIDs in this document, bits 64 and 65 of the UUID (bits 0 and 1 of octet 8) MUST be set to 1 and 0 as specified in row 2 of Table 1. Accordingly, all bit and field layouts avoid the use of these bits.

## 4.2. Version Field
The version number is in the most significant 4 bits of octet 6 (bits 48 through 51 of the UUID).

Table 2 lists all of the versions for this UUID variant 10xx specified in this document.

Table 2: UUID Variant 10xx Versions Defined by This Specification
```
MSB0	MSB1	MSB2	MSB3	Version	Description
0	0	0	0	0	Unused.
0	0	0	1	1	The Gregorian time-based UUID specified in this document.
0	0	1	0	2	Reserved for DCE Security version, with embedded POSIX UUIDs.
0	0	1	1	3	The name-based version specified in this document that uses MD5 hashing.
0	1	0	0	4	The randomly or pseudorandomly generated version specified in this document.
0	1	0	1	5	The name-based version specified in this document that uses SHA-1 hashing.
0	1	1	0	6	Reordered Gregorian time-based UUID specified in this document.
0	1	1	1	7	Unix Epoch time-based UUID specified in this document.
1	0	0	0	8	Reserved for custom UUID formats specified in this document.
1	0	0	1	9	Reserved for future definition.
1	0	1	0	10	Reserved for future definition.
1	0	1	1	11	Reserved for future definition.
1	1	0	0	12	Reserved for future definition.
1	1	0	1	13	Reserved for future definition.
1	1	1	0	14	Reserved for future definition.
1	1	1	1	15	Reserved for future definition.
```
An example version/variant layout for UUIDv4 follows the table where "M" represents the version placement for the hexadecimal representation of 0x4 (0b0100) and the "N" represents the variant placement for one of the four possible hexadecimal representation of variant 10xx: 0x8 (0b1000), 0x9 (0b1001), 0xA (0b1010), 0xB (0b1011).

```
00000000-0000-4000-8000-000000000000
00000000-0000-4000-9000-000000000000
00000000-0000-4000-A000-000000000000
00000000-0000-4000-B000-000000000000
xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
Figure 5: UUIDv4 Variant Examples
```
It should be noted that the other remaining UUID variants found in Table 1 leverage different sub-typing or versioning mechanisms. The recording and definition of the remaining UUID variant and sub-typing combinations are outside of the scope of this document.

## 5.7. UUID Version 7
UUIDv7 features a time-ordered value field derived from the widely implemented and well-known Unix Epoch timestamp source, the number of milliseconds since midnight 1 Jan 1970 UTC, leap seconds excluded. Generally, UUIDv7 has improved entropy characteristics over UUIDv1 (Section 5.1) or UUIDv6 (Section 5.6).

UUIDv7 values are created by allocating a Unix timestamp in milliseconds in the most significant 48 bits and filling the remaining 74 bits, excluding the required version and variant bits, with random bits for each new UUIDv7 generated to provide uniqueness as per Section 6.9. Alternatively, implementations MAY fill the 74 bits, jointly, with a combination of the following subfields, in this order from the most significant bits to the least, to guarantee additional monotonicity within a millisecond:

An OPTIONAL sub-millisecond timestamp fraction (12 bits at maximum) as per Section 6.2 (Method 3).
An OPTIONAL carefully seeded counter as per Section 6.2 (Method 1 or 2).
Random data for each new UUIDv7 generated for any remaining space.
Implementations SHOULD utilize UUIDv7 instead of UUIDv1 and UUIDv6 if possible.

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           unix_ts_ms                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          unix_ts_ms           |  ver  |       rand_a          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|var|                        rand_b                             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            rand_b                             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Figure 11: UUIDv7 Field and Bit Layout
```

**unix_ts_ms:**
48-bit big-endian unsigned number of the Unix Epoch timestamp in milliseconds as per Section 6.1. Occupies bits 0 through 47 (octets 0-5).

**ver:**
The 4-bit version field as defined by Section 4.2, set to 0b0111 (7). Occupies bits 48 through 51 of octet 6.

**rand_a:**
12 bits of pseudorandom data to provide uniqueness as per Section 6.9 and/or optional constructs to guarantee additional monotonicity as per Section 6.2. Occupies bits 52 through 63 (octets 6-7).

**var:**
The 2-bit variant field as defined by Section 4.1, set to 0b10. Occupies bits 64 and 65 of octet 8.

**rand_b:**
The final 62 bits of pseudorandom data to provide uniqueness as per Section 6.9 and/or an optional counter to guarantee additional monotonicity as per Section 6.2. Occupies bits 66 through 127 (octets 8-15).

### 6.1. Timestamp Considerations
UUID timestamp source, precision, and length were topics of great debate while creating UUIDv7 for this specification. Choosing the right timestamp for your application is very important. This section will detail some of the most common points on this issue.

**Reliability:**
Implementations acquire the current timestamp from a reliable source to provide values that are time ordered and continually increasing. Care must be taken to ensure that timestamp changes from the environment or operating system are handled in a way that is consistent with implementation requirements. For example, if it is possible for the system clock to move backward due to either manual adjustment or corrections from a time synchronization protocol, implementations need to determine how to handle such cases. (See "Altering, Fuzzing, or Smearing" below.)

**Source:**
UUIDv1 and UUIDv6 both utilize a Gregorian Epoch timestamp, while UUIDv7 utilizes a Unix Epoch timestamp. If other timestamp sources or a custom timestamp Epoch are required, UUIDv8 MUST be used.

**Sub-second Precision and Accuracy:**
Many levels of precision exist for timestamps: milliseconds, microseconds, nanoseconds, and beyond. Additionally, fractional representations of sub-second precision may be desired to mix various levels of precision in a time-ordered manner. Furthermore, system clocks themselves have an underlying granularity, which is frequently less than the precision offered by the operating system. With UUIDv1 and UUIDv6, 100 nanoseconds of precision are present, while UUIDv7 features a millisecond level of precision by default within the Unix Epoch that does not exceed the granularity capable in most modern systems. For other levels of precision, UUIDv8 is available. Similar to Section 6.2, with UUIDv1 or UUIDv6, a high-resolution timestamp can be simulated by keeping a count of the number of UUIDs that have been generated with the same value of the system time and using that count to construct the low order bits of the timestamp. The count of the high-resolution timestamp will range between zero and the number of 100-nanosecond intervals per system-time interval.

**Length:**
The length of a given timestamp directly impacts how many timestamp ticks can be contained in a UUID before the maximum value for the timestamp field is reached. Take care to ensure that the proper length is selected for a given timestamp. UUIDv1 and UUIDv6 utilize a 60-bit timestamp valid until 5623 AD; UUIDv7 features a 48-bit timestamp valid until the year 10889 AD.

**Altering, Fuzzing, or Smearing:**
Implementations MAY alter the actual timestamp. Some examples include security considerations around providing a real-clock value within a UUID to 1) correct inaccurate clocks, 2) handle leap seconds, or 3) obtain a millisecond value by dividing by 1024 (or some other value) for performance reasons (instead of dividing a number of microseconds by 1000). This specification makes no requirement or guarantee about how close the clock value needs to be to the actual time. If UUIDs do not need to be frequently generated, the UUIDv1 or UUIDv6 timestamp can simply be the system time multiplied by the number of 100-nanosecond intervals per system-time interval.

**Padding:**
When timestamp padding is required, implementations MUST pad the most significant bits (leftmost) with data. An example for this padding data is to fill the most significant, leftmost bits of a Unix timestamp with zeroes to complete the 48-bit timestamp in UUIDv7. An alternative approach for padding data is to fill the most significant, leftmost bits with the number of 32-bit Unix timestamp rollovers after 2038-01-19.

**Truncating:**
When timestamps need to be truncated, the lower, least significant bits MUST be used. An example would be truncating a 64-bit Unix timestamp to the least significant, rightmost 48 bits for UUIDv7.

**Error Handling:**
If a system overruns the generator by requesting too many UUIDs within a single system-time interval, the UUID service can return an error or stall the UUID generator until the system clock catches up and MUST NOT knowingly return duplicate values due to a counter rollover. Note that if the processors overrun the UUID generation frequently, additional Node IDs can be allocated to the system, which will permit higher speed allocation by making multiple UUIDs potentially available for each timestamp value. Similar techniques are discussed in Section 6.4.

## 6.2. Monotonicity and Counters
Monotonicity (each subsequent value being greater than the last) is the backbone of time-based sortable UUIDs. Normally, time-based UUIDs from this document will be monotonic due to an embedded timestamp; however, implementations can guarantee additional monotonicity via the concepts covered in this section.

Take care to ensure UUIDs generated in batches are also monotonic. That is, if one thousand UUIDs are generated for the same timestamp, there should be sufficient logic for organizing the creation order of those one thousand UUIDs. Batch UUID creation implementations MAY utilize a monotonic counter that increments for each UUID created during a given timestamp.

For single-node UUID implementations that do not need to create batches of UUIDs, the embedded timestamp within UUIDv6 and UUIDv7 can provide sufficient monotonicity guarantees by simply ensuring that timestamp increments before creating a new UUID. Distributed nodes are discussed in Section 6.4.

Implementations SHOULD employ the following methods for single-node UUID implementations that require batch UUID creation or are otherwise concerned about monotonicity with high-frequency UUID generation.

**Fixed Bit-Length Dedicated Counter (Method 1):**
Some implementations allocate a specific number of bits in the UUID layout to the sole purpose of tallying the total number of UUIDs created during a given UUID timestamp tick. If present, a fixed bit-length counter MUST be positioned immediately after the embedded timestamp. This promotes sortability and allows random data generation for each counter increment. With this method, the rand_a section (or a subset of its leftmost bits) of UUIDv7 is used as a fixed bit-length dedicated counter that is incremented for every UUID generation. The trailing random bits generated for each new UUID in rand_b can help produce unguessable UUIDs. In the event that more counter bits are required, the most significant (leftmost) bits of rand_b MAY be used as additional counter bits.

**Monotonic Random (Method 2):**
With this method, the random data is extended to also function as a counter. This monotonic value can be thought of as a "randomly seeded counter" that MUST be incremented in the least significant position for each UUID created on a given timestamp tick. UUIDv7's rand_b section SHOULD be utilized with this method to handle batch UUID generation during a single timestamp tick. The increment value for every UUID generation is a random integer of any desired length larger than zero. It ensures that the UUIDs retain the required level of unguessability provided by the underlying entropy. The increment value MAY be 1 when the number of UUIDs generated in a particular period of time is important and guessability is not an issue. However, incrementing the counter by 1 SHOULD NOT be used by implementations that favor unguessability, as the resulting values are easily guessable.

**Replace Leftmost Random Bits with Increased Clock Precision (Method 3):**
For UUIDv7, which has millisecond timestamp precision, it is possible to use additional clock precision available on the system to substitute for up to 12 random bits immediately following the timestamp. This can provide values that are time ordered with sub-millisecond precision, using however many bits are appropriate in the implementation environment. With this method, the additional time precision bits MUST follow the timestamp as the next available bit in the rand_a field for UUIDv7.

To calculate this value, start with the portion of the timestamp expressed as a fraction of the clock's tick value (fraction of a millisecond for UUIDv7). Compute the count of possible values that can be represented in the available bit space, 4096 for the UUIDv7 rand_a field. Using floating point or scaled integer arithmetic, multiply this fraction of a millisecond value by 4096 and round down (toward zero) to an integer result to arrive at a number between 0 and the maximum allowed for the indicated bits, which sorts monotonically based on time. Each increasing fractional value will result in an increasing bit field value to the precision available with these bits.

For example, let's assume a system timestamp of 1 Jan 2023 12:34:56.1234567. Taking the precision greater than 1 ms gives us a value of 0.4567, as a fraction of a millisecond. If we wish to encode this as 12 bits, we can take the count of possible values that fit in those bits (4096 or 212), multiply it by our millisecond fraction value of 0.4567, and truncate the result to an integer, which gives an integer value of 1870. Expressed as hexadecimal, it is 0x74E or the binary bits 0b011101001110. One can then use those 12 bits as the most significant (leftmost) portion of the random section of the UUID (e.g., the rand_a field in UUIDv7). This works for any desired bit length that fits into a UUID, and applications can decide the appropriate length based on available clock precision; for UUIDv7, it is limited to 12 bits at maximum to reserve sufficient space for random bits.

The main benefit to encoding additional timestamp precision is that it utilizes additional time precision already available in the system clock to provide values that are more likely to be unique; thus, it may simplify certain implementations. This technique can also be used in conjunction with one of the other methods, where this additional time precision would immediately follow the timestamp. Then, if any bits are to be used as a clock sequence, they would follow next.

The following sub-topics cover issues related solely to creating reliable fixed bit-length dedicated counters:

**Fixed Bit-Length Dedicated Counter Seeding:**
Implementations utilizing the fixed bit-length counter method randomly initialize the counter with each new timestamp tick. However, when the timestamp has not increased, the counter is instead incremented by the desired increment logic. When utilizing a randomly seeded counter alongside Method 1, the random value MAY be regenerated with each counter increment without impacting sortability. The downside is that Method 1 is prone to overflows if a counter of adequate length is not selected or the random data generated leaves little room for the required number of increments. Implementations utilizing fixed bit-length counter method MAY also choose to randomly initialize a portion of the counter rather than the entire counter. For example, a 24-bit counter could have the 23 bits in least significant, rightmost position randomly initialized. The remaining most significant, leftmost counter bit is initialized as zero for the sole purpose of guarding against counter rollovers.

**Fixed Bit-Length Dedicated Counter Length:**
Select a counter bit-length that can properly handle the level of timestamp precision in use. For example, millisecond precision generally requires a larger counter than a timestamp with nanosecond precision. General guidance is that the counter SHOULD be at least 12 bits but no longer than 42 bits. Care must be taken to ensure that the counter length selected leaves room for sufficient entropy in the random portion of the UUID after the counter. This entropy helps improve the unguessability characteristics of UUIDs created within the batch.
The following sub-topics cover rollover handling with either type of counter method:

**Counter Rollover Guards:**
The technique from "Fixed Bit-Length Dedicated Counter Seeding" above that describes allocating a segment of the fixed bit-length counter as a rollover guard is also helpful to mitigate counter rollover issues. This same technique can be used with monotonic random counter methods by ensuring that the total length of a possible increment in the least significant, rightmost position is less than the total length of the random value being incremented. As such, the most significant, leftmost bits can be incremented as rollover guarding.

**Counter Rollover Handling:**
Counter rollovers MUST be handled by the application to avoid sorting issues. The general guidance is that applications that care about absolute monotonicity and sortability should freeze the counter and wait for the timestamp to advance, which ensures monotonicity is not broken. Alternatively, implementations MAY increment the timestamp ahead of the actual time and reinitialize the counter.
Implementations MAY use the following logic to ensure UUIDs featuring embedded counters are monotonic in nature:

Compare the current timestamp against the previously stored timestamp.
If the current timestamp is equal to the previous timestamp, increment the counter according to the desired method.
If the current timestamp is greater than the previous timestamp, re-initialize the desired counter method to the new timestamp and generate new random bytes (if the bytes were frozen or being used as the seed for a monotonic counter).

**Monotonic Error Checking:**
Implementations SHOULD check if the currently generated UUID is greater than the previously generated UUID. If this is not the case, then any number of things could have occurred, such as clock rollbacks, leap second handling, and counter rollovers. Applications SHOULD embed sufficient logic to catch these scenarios and correct the problem to ensure that the next UUID generated is greater than the previous, or they should at least report an appropriate error. To handle this scenario, the general guidance is that the application MAY reuse the previous timestamp and increment the previous counter method.

## 6.9. Unguessability
Implementations SHOULD utilize a cryptographically secure pseudorandom number generator (CSPRNG) to provide values that are both difficult to predict ("unguessable") and have a low likelihood of collision ("unique"). The exception is when a suitable CSPRNG is unavailable in the execution environment. Take care to ensure the CSPRNG state is properly reseeded upon state changes, such as process forks, to ensure proper CSPRNG operation. CSPRNG ensures the best of Sections 6.7 and 8 are present in modern UUIDs.

Further advice on generating cryptographic-quality random numbers can be found in [RFC4086], [RFC8937], and [RANDOM].