# Baseload.DB
High Performance Schemaless In-Memory JSON Document Database Server

# Essential Features
* fast lookups - usage of templated in-memory b+ tree data structure
* no special drivers - exposure of rest interface
* small footprint - single-threaded multiplexed http server
* few dependencies - coded mainly from scratch in vanilla c/c++
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
with a decent amount of diligence and care and has been tested thoroughly as far as possible. This software will contain
bugs and some things might be poorly designed. Both applies (more or less) to any other software as well. Though I have
spent a lot of time in the past to make this piece of software what it is, it has always been a private
project parallel to my fulltime employments. Please understand that du to this reason maintainance might sometimes be
a bit slow and sluggish, I will try my very best!

This software does not aim to be extremely portable, instead it is designed to run under linux providing
a recent version of g++ that is compatible with C++ 20. The g++ version that has been used for development
is g++ 9.3.0. The reason for not making it highly portable in the first place is to save development time,
and second it is assumed to run on *servers* that in most cases run a linux distribution. It is mandatory
that this linux version (or the c library) supports the epoll system calls.

# Storage Engine
As a data structure, baseload implements a templated in-memory b+ tree, which serves as a key value store.
Both variants are implemented, the unique key to value mapping (corresponding to std::map) as well as
the one key to many values mapping (corresponding to std::multimap). In contrast to an ordinary b tree,
b+ trees hold their data exclusively in leaf nodes, which on the one hand allows for interconnecting
the leaf nodes in order to iterate them like a linked list and optimize for range queries, 
and on the other hand enables what is commonly
known as bulk loading meaning that key value pairs are read from a continuous stream while the tree is
built on top of the leaf-level. This avoids costly insertions during loading and is thus very efficient.

Though the implementation is templated, at the moment baseload is employing only the specific mapping
from std::string to json object as it is commonly used in document or no-sql databases. For all native
types, standard library containers, and classes specific to baseload like json object and json array, 
specialized serializers are implemented. Serialization is realized by binary reading from std::istream
and writing to std::ostream. Though currently only the document database is implemented, the road
is paved for implementing key value stores allowing for common native types.

# Transport Layer
Instead of shipping a set of drivers for several programming languages, baseload relies on the unified
http rest api approach. The http server is implemented in a single thread using the linux epoll multiplexing
interface, which allows for asynchroneous handling of multiple clients. The epoll instance is also connected
to a timer descriptor to enable regular events and to a signal descriptor to handle signals and terminate
the server gracefully in case of a received sigterm or sigkill signal. Incoming http requests are forwarded 
to the api, which performs tasks based on services that can be injected into the http server. In the current 
version, there are only two services available, which are i) the storage engine, and ii) the user management.
Regarding authorization, baseload implements only simple http basic authorization and there is no possibility
to set user permissions on specific documents, meaning that all users can read and modify all documents.

# Haproxy

# Logrotate

# Usage

```
jonas@DESKTOP-EE4KNAM:/mnt/c/Home/Native/Baseload.DB$ ./bin/database.app
Usage: ./bin/database.app [-v] [-d] [-c <config>].
         -v : verbose
         -d : daemon
         -c <config> : configuration in json format
```

# API

## Routes
* POST /insert: insert a document - requires plain json document in request body
* POST /remove: remove a document - requires json wrapped id in request body
* POST /find: find a document - requires json wrapped id in request body
* GET /keys: retrieve all keys
* GET /dump: retrieve a complete json dump

### Insert

### Remove

### Find





