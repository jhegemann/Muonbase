<p align="center">                                    
         <img src="https://user-images.githubusercontent.com/27009499/149123099-31945306-df89-46eb-8ba2-c69ee5c31a67.png" />
</p>

# Description
General Purpose Schemaless Persistent In-Memory JSON Document Database Server

# Essentials
* Fast lookups - Templated in-memory b+ tree data structure
* No special drivers - Http rest interface
* Small footprint - Single-threaded epoll http server
* Few dependencies - Coded from scratch in vanilla c/c++
* Overseeable codebase - Easy to understand and fully transparent
* Seamless integration - Json as data exchange format
* Persistence - Tree snapshot and write-ahead-log on disk
* Automated testing - Load scenario with random inserts and erasures

# Upcoming
* Queries
* Indexing
* Sharding
* Encryption
* Compression

# Disclaimer
Muonbase is far from being perfect and it explicitly does not claim to be so. However, it is written
with a decent amount of diligence and care and has been tested thoroughly as far as possible. Muonbase contains
and will contain bugs and some things might be poorly designed. Both applies (more or less) to any other software as well. 
Though it took years to make Muonbase what it is, it has always been a private
project parallel to fulltime employments. Please understand that therefore maintainance might sometimes be
a bit slow.

Muonbase does not aim to be extremely portable, instead it is designed to run within a common linux distribution.
It can be build with gcc and c++-20 standard. The g++ version that is currently in use for development
is g++ 9.3.0. The reason for not making it highly portable in the first place was to save development time.
In addition, it is assumed to run on *servers* that in most cases run a common linux distribution anyway. 
It is mandatory that epoll system calls are supported, but that is the normal case.

Muonbase is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and limitations under the License. Don't use Muonbase
as a single source of truth.

# SSL
Muonbase supports only plain http transport and does not implement ssl. In order to set up secure endpoints it 
is recommended to bind the server locally and let e.g. haproxy (or another suitable proxy server) do the ssl termination. 
This is easy, secure, and performant.

# LOG
Logs are either extremely verbose or totally absent. If you need logs e.g. for debugging purpose, 
either start the server in foreground and observe what happens on the standard output, 
or make sure you use e.g. logrotate to automatically rotate your logfiles.

# Build
Building is currently being performed with `g++-9.3.0` and `c++-20`. The only dependency so far is `openssl`, where you need
both the binary package `openssl` as well as the header files in `openssl-dev`. To build, proceed as normal with
```
make clean && make
```
The compiler is configured to give all warnings via `-Wall` and to be `-pedantic`; optimization level `-O3`is applied.

# Usage
Two binaries are produced by the makefile, which are (i) muonbase-server.app, and (ii) muonbase-client.app. Binary (i) runs the 
database server, which is used as follows
```
user@linux-machine:/home/muonbase$ ./bin/muonbase-server.app -h
Usage: muonbase-server.app [-h] [-v] [-d] [-c <config>]
         -h: help
         -v: verbose
         -d: daemonize
         -c <file>: configuration (mandatory)
```
The configuration file is a mandatory parameter. All other parameters and paths will be defined *in* this configuration file.
The option `-d` will run the server in background; option `-v` will switch on logging.

Binary (ii) runs automated tests against a running database server and is used as follows
```
user@linux-machine:/home/muonbase$ ./bin/muonbase-client.app -h
Usage: muonbase-client.app [-h] [-n <threads>] [-t] [-i <ip>] [-p <port>] [-o <order>] [-c <cycles>]
         -h: help
         -t: test
         -n <threads>: threads
         -i <ip>: ip
         -p <port>: port
         -o <order>: order
         -c <cycles>: cycles
```
Note that, when `-t` is omitted, the database client will just check if the database server is available on the specified ip and port.
Pass `-t` to run the randomized testing procedure and feel free to adjust `-o`, which is the number of initial database inserts,
and `-c`, which is the repetition number of a combined insert-erase operation.

# Configuration
```
{
  "ip": "127.0.0.1",
  "port": "8260",
  "dbPath": "./data/muonbase-storage.db",
  "userPath": "./config/muonbase-user.json",
  "logPath": "./muonbase-server.log",
  "workingDirectory": "./"
}
```
The configuration file is in a json format and it is mandatory for the database server to run. 
Tune the parameters as you want, but keep a few things in mind: (i) binding
to ip 0.0.0.0 is not recommended, since the transport layer does not support ssl, (ii) binding to
ports below 1024 is not recommended since this requires root privileges and is therefore by design not secure. 
The data and user paths can be chosen freely as well as the log path. Make sure permissions in particular 
of the users file are correct, such that it can be accessed properly by the database server. 
The working directory will only be used if the daemon command line option is activated, 
i.e. the server runs in background. If you run the server in background, 
you should prefer absolute paths over relative paths in the configuration file.

# Users
The user management is not dynamic yet, so in order to add a user you have to manually edit the users file
```
{
  "root": "9af15b336e6a9619928537df30b2e6a2376569fcf9d7e773eccede65606529a0"
}
```
which is, like the general configuration, in a json format. Hashed passwords are SHA256 and can be generated 
e.g. on the command line using `echo -n 'password' | sha256sum`. Don't forget to shred your shell history after doing that.
Some shells don't log commands to history if they are prefixed with a space. Regarding authorization, Muonbase by now implements 
only simple http basic authorization and there is no possibility to set user permissions on specific documents, 
meaning that all users can read and modify all documents in one collection. If you need user specific collections, scale 
horizontally and launch one new database server per user.

# API
There are three POST routes, which essentially mirror the c++ map interface, where you can insert, erase and find.
To insert, just present a plain json object in the request body, as a response you will receive the id of the inserted document.
To erase, present an id wrapped in a json object in the request body, as a response you will receive the same id back.
To find a document, do the same as for erasure, just change the route. You can also just GET an array with all keys or even
GET a complete json image.


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
