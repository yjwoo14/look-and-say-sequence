# look-and-say-sequence

## Requirement 
c++11

## How it works
The main idea in this implementation is generating a partial sequence of numbers as soon as we possible. For example, let's say the i'th number starts with 1112. We already know that the i+1'th number starts with 31 without knowing the rest of the i'th number.

`Reader` reads a partial sequence of the i'th number and passes the generated partial sequence to the next stage. More precisely, it gets each digit of the i'th number in the order while maintaining two variables, `last` the last digit the reader receives and `count` the number of digits in groups of the same digit. If the new given digit is the same with `last`, simply increase `count` by 1. Otherwise, read off a partial sequence by saying `count` and `last` (push theses to the next stage), and then update `count` and `last`.

To generate the N'th number, N-1 `Reader`s are created and linked in linearly. The last reader says the N'th number by pushing the first number to the first reader.

## 한국어 설명
https://yjwoo14.wordpress.com/2017/02/01/개미수열-look-and-say-sequence/
