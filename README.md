# Systems Programming Assignment 2
Georgios Alexandros Vasilakopoulos

1115202000018

### Compilation: ```make```

### Execution

- #### poller: ```./poller [portnum] [numWorkerthreads] [bufferSize] [poll-log] [poll-stats]```

- #### pollSwayer: ```./pollSwayer [serverName] [portNum] [inputFile.txt]```






## ***SERVER***

### *Master Thread*

Upon executing ```poller``` with the specified arguments, the *master thread*:

- Creates ```[numWorkerthreads]``` worker threads.

- Creates a main socket and binds it to the port specified by ```[portnum]```.

- Initializes a FIFO queue structure into which the client sockets will be stored.

Once the *master thread* accepts a connection with a client:

- It creates a new socket dedicated to that client and it inserts it into the queue, so that one of the worker threads may serve the client. The queue is protected by a mutex lock so that only one thread at a time may access the structure.

- If the queue has reached its full capacity (specified by ```[bufferSize]```), the *master thread* will wait on the condition variable ```nonFullCondition```. This condition variable will be signaled by a worker thread, once it removes a socket from the queue.

- After the *master thread* inserts a new socket into the queue, it signals the condition variable ```nonEmptyCondition```, so that a single worker thread can wake up, if they were waiting on the condition variable.

When the *master thread* receives a SIGINT signal:

- After seizing the mutex, it sets the ```stopFlag``` variable to 1, so that the *worker threads* are notified that they must exit.

- It broadcasts the ```nonEmptyCondition``` so that all of the waiting *worker threads* can wake up and exit.

- It waits for the ```worker threads``` to exit, through ```pthread_join()```

- It stores the voting results on the files ```[poll-log]``` and ```[poll-stats]```.

- It destructs the queue and record structure and exits gracefully.





### *Worker Threads*

Every worker thread, upon creation, disables the SIGINT signal. The *master thread* will **indirectly** notify the worker threads to terminate, through ```stopFlag```

On each serve cycle, a *worker thread*:

1. Locks the mutex 
2. Checks whether ```stopFlag``` is set. If so, it exits
3. Checks the size of the queue. If it is empty, it waits on the condition variable ```nonEmptyCondition```. *Worker threads* that wake up by a signal on ```nonEmptyCodition``` have to re-check whether ```stopFlag``` is set before proceeding.
4. Fetches a socket from the queue
5. Releases the mutex

After step 5, a worker thread can safely start serving the client:

6. Sends message "SEND NAME PLEASE\n"
7. Reads response until '\n' is found in the stream. Response is stored in a buffer ```name```.
8. Sends message "SEND VOTE PLEASE\n"
9. Reads response until '\n' is found. Response is stored in ```party```
10. While holding ```recordMutex```, it checks whether ```name``` has already voted. If so, it sends "ALREADY VOTED\n". Otherwise, "VOTE FOR PARTY \<party\> RECORDED" is sent.
11. Closes the socket.

**Notice that the verification of vote uniqueness happens after both ```name``` and ```party``` have been read from the stream**. Therefore, the server will always send exactly three messages to each client: 

- SEND NAME PLEASE
- SEND VOTE PLEASE
- VOTE FOR PARTY \<party\> RECORDED / ALREADY VOTED





### *Management of votes through ```votersRecord```*