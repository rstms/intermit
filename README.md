# intermit

Testing tool used with djb's tcpserver to introduce intermittent delays in server output


See ucspi-tcp documentaiton at [http://cr.yp.to/ucspi-tcp.html](http://cr.yp.to/ucspi-tcp.html)

 - usage is similar to recordio

Usage: intermit [-pPERCENT] [-dDELAY] [-sSEED] prog

intermit runs prog. Optionally, a percentage of the reads of prog's output will be delayed by the delay value.

## Parameters
```
-p percent of reads which will be delayed
-d delay in milliseconds
-s random seed 
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

The output delay is triggered upon each read system call, so it can appear at arbitrary locations in the data stream.

