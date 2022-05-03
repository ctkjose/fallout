#  Functions

Functions are self-contained chunks of code that perform a specific job. Functions help you divide your program into small reusable pieces of code.

A function has a **name** that identifies what it does, and this name is used to “call” the function to perform its task when needed.

A function is an actual data type.

```
function doThis(v="", code = 5){
    var i = code + 1.5;
    i = 12.5 + i;
    object.print("Inside my function");
    object.print(v);
    object.print(i);
}

doThis(name);
```

A closure is a type of function that has no name (is an anonymous function).

```
var fn = function(){
    object.print("Hello Jose");
};
```
