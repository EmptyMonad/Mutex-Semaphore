The only thing I was unable to do was get read/write to print properly beneath the buffer.
Likewise, the tail -1 on consumption is a little buggy.

This program requires 6 arguments. The execution, the run time, the max run time, the number of producer threads, the number of consumer threads, and 1/0 for buffer printing.

Eg. ./a.out 5 10 5 5 1