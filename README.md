# Muonbase
General Purpose High Performance Schemaless Persistent In-Memory JSON Document Database Server

# Essential Features
* fast lookups - usage of templated in-memory b+ tree data structure
* no special drivers - exposure of rest interface
* small footprint - single-threaded epoll http server
* few dependencies - coded from scratch in vanilla c/c++
* overseeable codebase - easy to understand and fully transparent
* seamless integration - use of json as data exchange format
* automated testing - load scenario with random inserts, lookups and removals

# Upcoming Features
* Persistence
* Sharding
* Encryption
* Compression
* Indexing
* Queries

# Disclaimer
This software is far from being perfect and it explicitly does not claim to be so. However, it is written
with a decent amount of diligence and care and has been tested thoroughly as far as possible. This software contains
and will contain bugs and some things might be poorly designed. Both applies (more or less) to any other software as well. 
Though I have spent years to make this piece of software what it is, it has always been a private
project parallel to my fulltime employments. Please understand that for that reason maintainance might sometimes be
a bit slow and sluggish, I will try my very best!

This software does not aim to be extremely portable, instead it is designed to run within a linux distribution.
It can be build with gcc and c++-20 standard. The g++ version that is currently in use for development
is g++ 9.3.0. The reason for not making it highly portable in the first place was to save development time.
In addition, it is assumed to run on *servers* that in most cases run a linux distribution anyway. It is mandatory
that this linux distribution (or the c library) supports the epoll system calls.

# Storage Engine
As a data structure, the software implements a templated in-memory b+ tree, which serves as a key value store.
Both variants are implemented, the unique key to value mapping (corresponding to std::map) as well as
the one key to many values mapping (corresponding to std::multimap). In contrast to an ordinary b tree,
b+ trees hold their data exclusively in leaf nodes, which on the one hand allows for (bidirectionally) interconnecting
the leaf nodes in order to iterate them like a linked list, and on the other hand enables what is commonly
known as bulk loading meaning that key value pairs are read from a continuous sorted stream while the tree is
built on top of the leaf level. This avoids costly insertions during loading and is thus very efficient.

Though the implementation is templated, at the moment the software is employing only the specific mapping
from std::string to json object as it is commonly used in document or no-sql databases. For all native
types, standard library containers, and specific classes like json object and json array, 
specialized serializers are implemented. Serialization is realized by binary reading from std::istream
and writing to std::ostream. Though currently only the document database (unique mapping from std::string to json object) 
is implemented, the road is in principle paved for implementing key value stores allowing for common native types.

# Transport Layer
Instead of shipping a set of drivers for several programming languages, the software relies on the unified
http rest api approach. The http server is implemented in a *single thread* using the linux epoll interface, 
which allows for highly performant asynchroneous handling of multiple clients. The epoll instance is also connected
to a timer descriptor to enable regular events and it is connected to a signal descriptor to handle signals and terminate
the server gracefully in case of a received sigterm or sigkill signal. Incoming http requests are forwarded 
to the http rest api, which performs tasks based on services that can be injected into the http server event loop. 
In the current version, there are only two services available, which are i) the storage engine, and ii) the user management.
Regarding authorization, the software implements only simple http basic authorization and there is no possibility
to set user permissions on specific documents, meaning that all users can read and modify all documents.

# SSL / Haproxy
This software supports only plain http transport and does not implement ssl. In order to set up secure endpoints it 
is recommended to bind the server locally and let e.g. haproxy (or another suitable proxy server) do the ssl termination. 
This is easy, secure, and performant.

# Log / Logrotate
At the current early stage, logging can only be (very) verbose or totally absent. If you
really need the logging, either start the server in foreground and observe what happens on the standard output, 
or make sure you use e.g. logrotate to avoid blocking your disk space with very large logfiles.

# Build
As already mentioned in the disclaimer, building is currently being performed with gcc 9.3.0 and c++-20. The only
dependency so far is openssl, which needs to be installed as well as the corresponding headers openssl-dev. To build
the projects, simply type
```
make clean && make
```
in a login shell.

# Usage
Two binaries are produced by the makefile, which are (i) database.app, and (ii) test.app. Binary (i) runs the 
database server if a suitable configuration file in a json format is provided, see section configuration:
```
user@linux-machine:/home/db$ ./bin/database.app
Usage: database.app [-h] [-v] [-d] [-c <config>]
         -h: help
         -v: verbose
         -d: daemon
         -c <file>: configuration (mandatory)
```
Binary (ii) runs automated tests against a running database server:
```
user@linux-machine:/home/db$ ./bin/test.app
Usage: test.app [-h] [-t] [-i <ip>] [-p <port>] [-o <order>] [-c <cycles>]
         -h: help
         -t: test
         -i <ip>: ip
         -p <port>: port
         -o <order>: order
         -c <cycles>: cycles
```

# Configuration
The following configuration file in json format is the standard configuration, 
```
{
  "ip": "127.0.0.1",
  "port": "8260",
  "dbPath": "./data/storage.db",
  "userPath": "./config/users.json",
  "logPath": "./server.log",
  "workingDirectory": "."
}
```
which is mandatory for the database server. Tune the parameters as you want, but keep a few things in mind: (i) binding
to ip 0.0.0.0 is not recommended, since the transport layer does not support ssl, (ii) do not bind to
ports below 1024 since this requires root privileges and is therefore not secure. The data and user paths
can be chosen freely as well as the log path. The working directory will only be used if the daemon command
line option is activated, i.e. the server will run in background. In general absolute paths are preferred
over relative paths in this configuration file.

# User Management
The user management is not dynamic yet, so in order to add a user you have to manually edit the users file,
```
{
  "root": "9af15b336e6a9619928537df30b2e6a2376569fcf9d7e773eccede65606529a0"
}
```
which is in a json format. Hashes have to be sha256 and can be generated e.g. on the command line.

# API

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




