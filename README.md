# intermit

Testing tool used with djb's tcpserver to introduce intermittent delays in server output


See the ucspi-tcp documentation at [http://cr.yp.to/ucspi-tcp.html](http://cr.yp.to/ucspi-tcp.html)

 - usage is similar to recordio

Usage: intermit [-pPERCENT] [-dDELAY] [-sSEED] [-v] prog

intermit runs prog. Optionally, a percentage of the reads of prog's output will be delayed by the delay value.


## Parameters
```
-p percent of reads which will be delayed (default is 0)
-d delay in milliseconds (default is 1 second delay)
-s random seed (default seeds to time()) 
-v verbose mode (emit "pause" and "resume" on standard error)
```

## Example:

If the server under test is started with this command line:

```
tcpserver 0 10101 serverbinary arg1 arg2
```

The following command will introduce 3 second delays on 50% of the output:
```
tcpserver 0 10101 intermit -p50 -d3000 serverbinary arg1 arg2
```

The output delay trigger condition is checked on each read system call, so it can occur at arbitrary locations in the data stream.

## Installation

Clone the repository, then issue the following commands in the project directory:
```
make 
sudo make install
```  

## Note

This is intended for debugging client errors or regression testing.  It has no business whatsoever in a production system.

