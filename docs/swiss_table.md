# What is Swiss Table?

This is a question I first was asking when I started this project.

Suggested by a friend I watched a video from cppcon about the the swiss table, and it took me a solid 3 days of dwelling over and watching the video multiple times to develop a mental model of how it works (the video can be found [here](https://www.youtube.com/watch?v=ncHmEUmJZf4)).

I will try my best now to explain the concept in as simple terms as possible. I'll break it down first -- why we need it, and then go into some of the high-level explanations of how it works.

## Why do we need it?

I'd say it's pretty common knowledge that hash tables are immensely common. Realistically, there isn't one universal implementation of them which applies to every use case, but there are a few set implementations which have their own advantages and disadvantages.

`std::unordered_map` is the most common implementation. It uses chaining to resolve collisions within the table, ultimately leading to a lot of memory overhead with cache misses due to working with pointers.

`boost::unordered_map` is a more efficient implementation of `std::unordered_map` which uses open addressing to resolve collisions, but it still has a lot of overhead due to the way it handles collisions.

This is what lead to the development of the swiss table. While a swiss table continues to use open addressing, it uses a new interesting approach to resolve the collisions. The technique is labelled as "grouping", and it allows for a much more efficient way to resolve collisions, while also allowing for a much more efficient way to store the metadata of the table.

## How does it work?

As mentioned, the swiss table still uses open addressing to resolve the collisions, but it is based around concepts of bucketing. In order to boost the efficiency of collision handling, a swiss table uses a vector of control bits of 1-byte each. These control bytes are set to be the first 7 bits of an item's hash, with a leading bit to check if a slot is empty. In doing so, we can almost instantaneously search 8-16 consecutive slots of the table to massively reduce our search space to only locations with collisions in both the leading 57 bits of the hash, and the remaining 7 bits.

Ultimately this is where the big optimisation comes from, allowing us to use bucketing techniques and bit manipulation to effectively 