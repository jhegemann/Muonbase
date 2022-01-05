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

# API
* POST /insert: insert a document - requires plain json document in request body
* POST /remove: remove a document - requires json wrapped id in request body
* POST /find: find a document - requires json wrapped id in request body
* GET /keys: retrieve all keys
* GET /dump: retrieve a complete json dump




