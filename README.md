# Client-Server Model through TCP

Assignment for class ```K24 Systems Programming```

### Compilation: ```make```

### Execution

- #### poller: ```./poller [portnum] [numWorkerthreads] [bufferSize] [poll-log] [poll-stats]```

- #### pollSwayer: ```./pollSwayer [serverName] [portNum] [inputFile.txt]```






## ***poller server***

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

- The votersRecord module introduces a few simple methods that provide convenient access to the votes record.

- The votersRecord *structure* is mainly consisted of a hashtable (aka map). During the insertion of a vote, a mapping is created that correlates the voter name and the party that they voted for. Through ```findPartyOfVoter```, one can query the hashtable structure and determine the party of a certain voter. One may also remove a vote from the file, through ```RemoveVote``` (even though this functionality is not utilized)

- The module also provides a method that prints the voter logs into the given file. This works by opening a file pointer and 'visiting' all of the mappings with a certain function that prints the mapping onto the file that is pointed by the global file pointer ```saveVoteFile```.

- Similarly, the vote stats are printed into the file. The process that is followed is the following:  
	1. Create a new hashtable that maps parties to an integer number, representing the number of votes received.
	2. Visit all of the vote mappings stored in the record and, for each one, increment the vote count of the corresponding party in the new hashtable.
	3. Insert the mappings of the new hashtable (party-votesCount) into a buffer and sort them based on the number of votes received.
	4. Print the voting stats into the given file in a decreasing order of votes.




## ***pollSwayer client***

The initial pollSwayer thread opens the given input file in read mode and initializes the global ```inputFilePointer```, through which the worker threads will read the votes.

Once created by the main thread, each worker thread:

- Reads a line from the file, by accessing ```inputFilePointer```. The pointer itself is protected by a mutex, in order to prevent corruption by simultaneous access.

- Initializes the server variables and attempts to connect to the server.

- If the connection is successful, each worker thread waits for the initial message "SEND NAME PLEASE" and then proceeds to send the name of the voter and the party. 

- Between each sent message, the thread awaits for the response of the server. If that were not the case, then the client would close the connection before the server could send the confirmation message, and that would cause a broken pipe error.



