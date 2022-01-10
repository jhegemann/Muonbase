# Muonbase
General Purpose Schemaless Persistent In-Memory JSON Document Database Server

# Essential Features
* fast lookups - templated in-memory b+ tree data structure
* no special drivers - http rest interface
* small footprint - single-threaded epoll http server
* few dependencies - coded from scratch in vanilla c/c++
* overseeable codebase - easy to understand and fully transparent
* seamless integration - json as data exchange format
* automated testing - load scenario with random inserts and erasures

# Upcoming Features
* Queries
* Indexing
* Persistence
* Sharding
* Encryption
* Compression

# Disclaimer
Muonbase is far from being perfect and it explicitly does not claim to be so. However, it is written
with a decent amount of diligence and care and has been tested thoroughly as far as possible. Muonbase contains
and will contain bugs and some things might be poorly designed. Both applies (more or less) to any other software as well. 
Though it took years to make Muonbase what it is, it has always been a private
project parallel to fulltime employments. Please understand that for that reason maintainance might sometimes be
a bit slow, I will try my very best!

Muonbase does not aim to be extremely portable, instead it is designed to run within a common linux distribution.
It can be build with gcc and c++-20 standard. The g++ version that is currently in use for development
is g++ 9.3.0. The reason for not making it highly portable in the first place was to save development time.
In addition, it is assumed to run on *servers* that in most cases run a common linux distribution anyway. It is mandatory
that this linux distribution supports the epoll system calls.

# Storage Engine
As a data structure, Muonbase implements a templated in-memory b+ tree, which serves as a key value store.
Both variants are implemented, the unique key to value mapping (corresponding to a map/dict) as well as
the one key to many values mapping (corresponding to a multimap). In contrast to an ordinary b tree,
b+ trees hold their data exclusively in leaf nodes, which on the one hand allows for (bidirectionally) interconnecting
the leaf nodes in order to iterate them like a linked list, and on the other hand enables what is commonly
known as bulk loading meaning that key value pairs are read from a continuous sorted stream while the tree is
built on top of the leaf level. This avoids costly insertions, which potentially lead to node splits,
during loading and is therefore efficient.

Though the implementation is templated, at the moment Muonbase is employing only the specific mapping
from string id to json object as it is commonly used in document or no-sql databases; the string id will be
a random id with 16 characters. For all native types, standard library containers, and specific classes like 
json object and json array, specialized binary serializers are implemented. The road is in principle 
paved for implementing key value stores allowing for common native types, if anyone wants to do that.

# Persistence
Muonbase keeps the whole set of json documents in-memory in a b+ tree. However, this does not mean that Muonbase
does not persist them to disk. Every insert or erase operation will be logged on disk in a binary sequential
journal, which happens before changes are applied to the in-memory structure. This way, it is guaranteed
that changes that are visible in-memory (and therefore also to other clients) are readily persisted on disk.
If the journal exceeds a certain size, it is rotated, i.e., it is renamed and never touched again.
Only *one* rotated journal can exist at a time. The database service checks regularly if such a closed journal exists; 
if yes, it will start the concurrent rollover sequence.

The rollover sequence is defined as follows: (i) if database file (sequence of binary key value pairs suitable for b+ tree 
bulk loading) is present, read it into memory, (ii) if there is a rotated journal, replay it and update the b+ tree that
has just been loaded into memory; or, if no b+ tree was present, create a new b+ tree and replay the journal into this new b+ tree,
(iii) persist the updated b+ tree to disk and exchange the resulting file with the last valid snapshot. All this can be performed
concurrently in a separate thread, since both the b+ tree snapshot as well as the rotated journal are static during normal
database operation, i.e. the server does not interact with them. When starting the database server, the above procedure is
performed synchroneously and always.

# Transport Layer
Instead of shipping a set of drivers for several programming languages, the software relies on the unified
http rest api approach. The http server is implemented in a *single thread* using the linux epoll interface, 
which allows for highly performant asynchroneous handling of many clients. The epoll instance is also connected
to a timer descriptor to enable regular events, and it is connected to a signal descriptor to handle signals and terminate
the server gracefully in case of a received sigterm or sigkill signal. Incoming http requests are, depending on their route, 
forwarded to the corresponding http rest api callback, which performs tasks based on a map of services that have been 
injected into the http server event loop. In the current version, there are only two services available, which are 
i) the storage engine, and ii) the user management. 

# SSL / Haproxy
This software supports only plain http transport and does not implement ssl. In order to set up secure endpoints it 
is recommended to bind the server locally and let e.g. haproxy (or another suitable proxy server) do the ssl termination. 
This is easy, secure, and performant.

# Log / Logrotate
At the current early stage, logging is either extremely verbose or totally absent. If you
need the logging e.g. for debugging purpose, either start the server in foreground and observe what happens on the standard output, 
or make sure you use e.g. logrotate to automatically rotate your logfiles.

# Build
Building is currently being performed with `g++-9.3.0` and `c++-20`. The only dependency so far is openssl, where you need
both the binary package `openssl` as well as the header files in `openssl-dev`
```
make clean && make
```
The compiler is configured to give all warnings via `-Wall` and to be `-pedantic`, and optimization level `-O3`is applied.

# Usage
Two binaries are produced by the makefile, which are (i) muonbase-server.app, and (ii) muonbase-client.app. Binary (i) runs the 
database server if a suitable configuration file in a json format is provided, see section configuration. The database server
is used as follows:
```
user@linux-machine:/home/muonbase$ ./bin/muonbase-server.app -h
Usage: muonbase-server.app [-h] [-v] [-d] [-c <config>]
         -h: help
         -v: verbose
         -d: daemonize
         -c <file>: configuration (mandatory)
```
The configuration file is a mandatory parameter. All other parameters and paths will be defined in the configuration file.
The option `-d` will run the server in the background with a working directory that can be specified in the configuration.

Binary (ii) runs automated tests against a running database server:
```
user@linux-machine:/home/muonbase$ ./bin/muonbase-client.app
Usage: muonbase-client.app [-h] [-t] [-i <ip>] [-p <port>] [-o <order>] [-c <cycles>]
         -h: help
         -t: test
         -i <ip>: ip
         -p <port>: port
         -o <order>: order
         -c <cycles>: cycles
```
Note that, when `-t` is ommitted, the database client will just check if the database server is available on specified ip and port.
Pass `-t` to run the randomized testing procedure and feel free to adjust the `-o`, which is the number of initial database inserts,
and `-c`, which is the repetition number of an insert and erase combination operation.

# Configuration
The json configuration file
```
{
  "ip": "127.0.0.1",
  "port": "8260",
  "dbPath": "./data/storage.db",
  "userPath": "./config/users.json",
  "logPath": "./server.log",
  "workingDirectory": "./"
}
```
is mandatory for the database server to run. Tune the parameters as you want, but keep a few things in mind: (i) binding
to ip 0.0.0.0 is not recommended, since the transport layer does not support ssl, (ii) do not bind to
ports below 1024 since this requires root privileges and is therefore not secure. The data and user paths
can be chosen freely as well as the log path. Make sure permissions of the users file are correct, such that it 
can be accessed properly by the service. The working directory will only be used if the daemon command
line option is activated, i.e. the server runs in background. If you run the server in background, 
you should prefer aboslute paths over relative paths in the configuration file.

# User Management
The user management is not dynamic yet, so in order to add a user you have to manually edit the users file
```
{
  "root": "9af15b336e6a9619928537df30b2e6a2376569fcf9d7e773eccede65606529a0"
}
```
which is, like the general configuration, in a json format. Hashed passwords are in SHA256 and can be generated 
e.g. on the command line using `echo -n 'password' | sha256sum`. Don't forget to shred your shell history after doing that.
Some shells don't log commands to history if it is prefix them with a space. Regarding authorization, Muonbase by now implements 
only simple http basic authorization and there is no possibility to set user permissions on specific documents, 
meaning that all users can read and modify all documents in one collection. If you need user specific collections, scale 
horizontally and launch one new database server per user.

# API
There are three POST routes, which essentially mirror the common c++ map interface, where you can insert, erase and find.
To insert just present a plain json object in the request body, as a response you will receive the id of the inserted document.
To erase present an id wrapped in a json object in the request body, as a response you will receive the same id back.
To find a document, do the same as for erasure, just change the route. You can also just GET a list with all keys or even
GET a complete json image, if you need that.


## Routes
* POST /insert
* POST /erase
* POST /find
* GET /keys
* GET /image

### Insert
#### Request
```
POST /insert HTTP/1.1
authorization: Basic cm9vdDowMDAw
content-length: 275
content-type: application/json

{"qRADfzL9qSZdWfCB":[true,0.481446,319226,null],"KneSOtkMNGxvUhH1":{"BrWUguL5y0ov17n3":728229,"VeGU6JjPxbWrWe79":null,"cMvIo2nbwkvcnMBe":0.691427,"2f4e7JvjxynQnotm":false},"j8O5fgYpvwRb38hy":null,"qmirthOPG2AyuSwD":792874,"PGjfooCv98HL1dTf":0.054929,"fE40YyX8iIgaQuXV":false}
```

#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 40
content-type: application/json
date: 20220106121227
server: muonbase/1

{"id":"dMxiajoFmCZirIiD","success":true}
```

### Erase
#### Request
```
POST /erase HTTP/1.1
authorization: Basic cm9vdDowMDAw
content-length: 25
content-type: application/json

{"id":"GI0xHlR9SHXpNPT9"}
```

#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 40
content-type: application/json
date: 20220106121227
server: muonbase/1

{"id":"GI0xHlR9SHXpNPT9","success":true}
```

### Find
#### Request
```
POST /find HTTP/1.1
authorization: Basic cm9vdDowMDAw
content-length: 25
content-type: application/json

{"id":"0jEdOcRMlgQeeuGe"}
```

#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 339
content-type: application/json
date: 20220106121227
server: muonbase/1

{"found":true,"document":{"fE40YyX8iIgaQuXV":false,"PGjfooCv98HL1dTf":0.451518,"qmirthOPG2AyuSwD":246816,"j8O5fgYpvwRb38hy":null,"KneSOtkMNGxvUhH1":{"2f4e7JvjxynQnotm":true,"cMvIo2nbwkvcnMBe":0.663296,"VeGU6JjPxbWrWe79":null,"BrWUguL5y0ov17n3":53522},"qRADfzL9qSZdWfCB":[false,0.133002,195159,null]},"id":"0jEdOcRMlgQeeuGe","success":true}
```
### Keys
#### Request
```
GET /keys HTTP/1.1
accept: */*
authorization: Basic cm9vdDowMDAw
content-type: application/json
host: 127.0.0.1:8260
user-agent: curl/7.68.0


```
#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 2442
content-type: application/json
date: 20220110111045
server: muonbase/1

{"keys":["0Uewy0VVq6iB6y6I","0WHjlUmQhGhDLJPa","0wBJ6tQqPPUmXQ12","124XEm2anguibxga","15MYuTCMN8bXH7fp","1qyVnKQWEBY9WYEz","4A56V27ovdvv9DOL","4Qdm5s8YsDZ9xyoS","5JRezsZAkCoPLFqV","5M9sSU7ksSzDelc9","6ecdEW01tzMWcBjH","707vMvcIkyLJqrkw","8705l1lYdJh4yNim","8CjPDS3hp7mnWLo9","8lffbuONUD520F4h","93KRBeiTXZYK6kpv","99jBoewiuMFyGcoP","9GWYbHM6JBTh2Zad","9kzOUGIC8leQSFcc","9tkUOCZQgfC16bjL","B3HukI72ojwsOhsB","B5el4SfLrv1UcGya","BazZGJ7VTi6lIE3j","BisFkgC0UIeDxxcG","BkXJk6Kcr0JGSZjI","BpHAsh55EM6yytqL","C2qEC0JbmTN0s1dc","DUxZC8fGvZbKfPwp","DpGqm0bQGGXUSgQ7","Ex6dapfK2jDttghY","FANs6OBa1pUTBQyO","FjPSf65MMEWT1o1u","FxKMzB3cDrRwi4Z5","HQ3riVxmcvwn9mWD","IWXjFkG5mU1csmq4","Ipepw456mgzsrH2S","IpzGjPoankgTCXGj","Isp1qawK9QMdWIzt","JI4kq9doPcZl8k5h","JSI8V9f1C3v1Yvru","K4BKOyJ7c2tJTEb8","KY1YfnKGBQ4dastv","KYJnEmB1L2CCvWRj","Kuhu1BSILze0yVOS","M2ZuPoWCqHL6GAfI","MXftdUipCT3XJWge","N1NvyI59Vfa3bAdh","NBDIgecdAqX4xPb0","NCBsawgooQ9CVIBR","NgJdiH04AkeYuCvj","NvgAZN8njpOMJjj8","O5cSKez2n9BvCOZ2","OZDwswTxtc2cDahM","PAf2sRe...
```
### Image
#### Request
```
GET /image HTTP/1.1
accept: */*
authorization: Basic cm9vdDowMDAw
content-type: application/json
host: 127.0.0.1:8260
user-agent: curl/7.68.0


```
#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 37683
content-type: application/json
date: 20220110110834
server: muonbase/1

{"PfeXz4v8sN1yw2h3":{"fE40YyX8iIgaQuXV":true,"PGjfooCv98HL1dTf":0.959832,"qmirthOPG2AyuSwD":427611,"j8O5fgYpvwRb38hy":null,"KneSOtkMNGxvUhH1":{"2f4e7JvjxynQnotm":true,"cMvIo2nbwkvcnMBe":0.614108,"VeGU6JjPxbWrWe79":null,"BrWUguL5y0ov17n3":92236},"qRADfzL9qSZdWfCB":[false,0.180922,89376,null]},"NvgAZN8njpOMJjj8":{"fE40YyX8iIgaQuXV":false,"PGjfooCv98HL1dTf":0.848913,"qmirthOPG2AyuSwD":694597,"j8O5fgYpvwRb38hy":null,"KneSOtkMNGxvUhH1":{"2f4e7JvjxynQnotm":true,"cMvIo2nbwkvcnMBe":0.726031,"VeGU6JjPxbWrWe79":null,"BrWUguL5y0ov17n3":959787},"qRADfzL9qSZdWfCB":[true,0.487173,571937,null]},"NgJdiH04AkeYuCvj":{"fE40YyX8iIgaQuXV":true,"PGjfooCv98HL1dTf":0.842632,"qmirthOPG2AyuSwD":444383,"j8O5fgYpvwRb38hy":null,"KneSOtkMNGxvUhH1":{"2f4e7JvjxynQnotm":false,"cMvIo2nbwkvcnMBe":0.449838,"VeGU6JjPxbWrWe79":null,"BrWUguL5y0ov17n3":916759},"qRADfzL9qSZdWfCB":[true,0.594556,67590,null]},"N1NvyI59Vfa3bAdh":{"fE40YyX8iIgaQuXV":true,"PGjfooCv98HL1dTf":0.196309,"qmirthOPG2AyuSwD":1027343,"j8O5fgYpvwRb38hy":null,"KneSOtkMNGxvUhH1":{"...
```

# Sloccount
```
user@linux-machine:/home/db$ sloccount source/ include/
Creating filelist for source
Creating filelist for include
Categorizing files.
Finding a working MD5 command....
Found a working MD5 command.
Computing results.


SLOC    Directory       SLOC-by-Language (Sorted)
3534    source          cpp=3534
2400    include         cpp=2111,ansic=289


Totals grouped by language (dominant language first):
cpp:           5645 (95.13%)
ansic:          289 (4.87%)




Total Physical Source Lines of Code (SLOC)                = 5,934
Development Effort Estimate, Person-Years (Person-Months) = 1.30 (15.57)
 (Basic COCOMO model, Person-Months = 2.4 * (KSLOC**1.05))
Schedule Estimate, Years (Months)                         = 0.59 (7.10)
 (Basic COCOMO model, Months = 2.5 * (person-months**0.38))
Estimated Average Number of Developers (Effort/Schedule)  = 2.19
Total Estimated Cost to Develop                           = $ 175,249
 (average salary = $56,286/year, overhead = 2.40).
SLOCCount, Copyright (C) 2001-2004 David A. Wheeler
SLOCCount is Open Source Software/Free Software, licensed under the GNU GPL.
SLOCCount comes with ABSOLUTELY NO WARRANTY, and you are welcome to
redistribute it under certain conditions as specified by the GNU GPL license;
see the documentation for details.
Please credit this data as "generated using David A. Wheeler's 'SLOCCount'."
```