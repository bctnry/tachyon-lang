# tachyon demo

requires go v1.24

## primitives

All integers must consists of only digits from `0` to `9`. All words prefixed by a single-quote `'` is considered to be word literal (e.g. `Factorial` executes the `Factorial` command, but `'Factorial` pushes the word `Factorial` onto the stack). Boolean literals are `TRUE` and `FALSE`. 

```
drop  ::  a b ...  -->  b ...
swap  ::  a b ...  -->  b a ...
dup   ::  a ...  -->  a a ...
over  ::  a b ...  -->  b a b ...
rot3  ::  a b c ...  -->  c a b ...
inc  ::  INT ...  -->  INT ...   // integer increment
dec  ::  INT ...  -->  INT ...   // integer decrement
+  ::  INT INT ...  -->  INT ...   // integer add
*  ::  INT INT ...  -->  INT ...   // integer multiply
<=  ::  INT INT ...  -->  BOOL ...   // integer comparison
==  ::  INT INT ...  -->  BOOL ...
if  ::  BOOL (a -> b) (a -> c) ...  -->  (b | c)   // condition
dstk  ::  a  -->  a   // dump stack; displays all the elements on the stack
#  ::  WORD a ...  -->  ...   // define in current environment
$  ::  WORD ...  -->  a ...   // retrieve from current environment
begin  ::  a  -->  a   // new environment page
end  ::  a  -->  a   // discard current environment page
[  ::  ...  -->  ESC ...   // put a dynamic array on top of the stack and enter escape mode
// escape mode:
// 1.  all words (except [ and ] ) become escaped and pushed to
//     the dynamic list on the top of the stack.
// 2.  each instance of [ in escape mode pushes an empty dynamic
//     list to the stack and increases the "escape count".
// 3.  each instance of ] seals the dynamic list on the top of
//     the stack, removes it from the stack and decreases the
//     "escape count". if after removing the sealed dynamic
//     list there's an open dynamic list underneath, the sealed
//     list is then pushed into that list.
// 4.  when the escape count reaches 0, we return from escape mode
//     to normal mode.
]  ::  ESC ...  -->  LIST ...   // seal an ESC into a proper list
```

Other primitives can be easily defined and implemented.

## Example

The following is an example of the factorial function:

```
'Factorial [
   'a swap #
   'a $ 0 ==
     [ 0 ]
	 [ 'a $ 1 ==
	   [ 1 ]
	   [ 'a $ dup dec Factorial * ]
	   if
	 ]
	 if
] #
```

One can then call it like e.g. `5 Factorial`.

