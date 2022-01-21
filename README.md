<p align="center">                                    
         <img src="https://user-images.githubusercontent.com/27009499/149123099-31945306-df89-46eb-8ba2-c69ee5c31a67.png" />
</p>

# Description
General Purpose Persistent In-Memory JSON Document Database Server

# Essentials
* CRUD performance - Templated in-memory B+ tree data structure
* Storage Persistence - Compact B+ tree snapshot and journal on disk
* Efficient transport - Epoll server
* No special drivers - HTTP REST interface
* Concise codebase - Easy to understand and fully transparent
* Few dependencies - Coded from scratch in vanilla C++
* Seamless integration - JSON as data exchange format
* Automated testing - Load scenario with random inserts and erasures

# Upcoming
* Queries
* Indexing
* Encryption
* Compression
* Sharding

# Disclaimer
Muonbase is far from being perfect and it explicitly does not claim to be so. However, it is written
with a decent amount of diligence and care and has been tested thoroughly as far as possible. Muonbase contains
and will contain bugs and some things might be poorly designed. Both applies (more or less) to any other software as well. 
Though it took years to make Muonbase what it is, it has always been a private
project parallel to fulltime employments. Please understand that therefore maintainance might sometimes be
a bit slow.

Muonbase does not aim to be extremely portable, instead it is designed to run within a common linux distribution.
It can be build with `gcc` and `c++-20` standard. The `g++` version that is currently in use for development
is `g++ 9.3.0`. The reason for not making it highly portable in the first place was to save development time.
In addition, it is assumed to run on *servers* that in most cases run a common linux distribution anyway. 
It is mandatory that epoll system calls are supported, but that is the typical case.

Muonbase is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and limitations under the License. Don't use Muonbase
as a single source of truth.

# SSL
Muonbase supports only plain HTTP transport and does not implement SSL. In order to set up secure endpoints it 
is recommended to bind the server locally and let e.g. `haproxy` (or another suitable proxy server) do the SSL termination. 
This is easy, secure, and performant.

# Build
Building is currently being performed with `g++-9.3.0` and `c++-20`. The only dependency so far is `openssl`, where you need
both the binary package `openssl` as well as the header files in `openssl-dev`. To build, proceed as normal with
```
make clean && make
```
The compiler is configured to give all warnings via `-Wall` and to be `-pedantic`; optimization level `-Ofast`is applied.

# Usage
Two binaries are produced by the makefile, which are (i) muonbase-server, and (ii) muonbase-client. Binary (i) runs the 
database server, which is used as follows
```
user@linux-machine:/home/muonbase$ ./bin/muonbase-server -h
Usage: muonbase-server [-h] [-v] [-d] [-c <config>]
         -h: help
         -v: verbose
         -d: daemonize
         -c <file>: configuration (mandatory)
```
The configuration file is a mandatory parameter. All other parameters and paths will be defined *in* this configuration file.
The option `-d` will run the server in background; option `-v` will switch on logging.

Binary (ii) runs automated tests against a running database server and is used as follows
```
user@linux-machine:/home/muonbase$ ./bin/muonbase-client -h
Usage: muonbase-client [-h] [-n <threads>] [-t] [-i <ip>] [-p <port>] [-o <order>] [-c <cycles>]
         -h: help
         -t: test
         -n <threads>: threads
         -i <ip>: ip
         -p <port>: port
         -o <order>: order
         -c <cycles>: cycles
```
Note that, when `-t` is omitted, the database client will just check if the database server is available on the specified ip and port.
Pass `-t` to run the randomized testing procedure and adjust `-o`, which is the number of initial database inserts,
and `-c`, which is the repetition number of a combined insert-erase operation.

# Logs
Logs are either extremely verbose or totally absent. If you need logs e.g. for debugging purpose, 
either start the server in foreground and observe what happens on the standard output, 
or make sure you use e.g. `logrotate` to automatically rotate your logfiles.

# Configuration
The configuration file is in a JSON format and it is mandatory for the database server to run. 
Tune the parameters as you want, but keep a few things in mind: (i) binding
to ip `0.0.0.0` is not recommended, since the transport layer does not support SSL, (ii) binding to
ports below `1024` is not recommended since this requires root privileges and is therefore by design not secure. 
The data and user paths can be chosen freely as well as the log path. Make sure permissions in particular 
of the users file are correct, such that it can be accessed properly by the database server. 
The working directory will only be used if the daemon command line option is activated, 
i.e. the server runs in background. If you run the server in background, 
you should prefer absolute paths over relative paths in the configuration file.

# Users
The user management is not dynamic, so in order to add a user you have to manually edit the users file, which is, 
like the general configuration, in a JSON format. Hashed passwords are SHA256 and can be generated 
e.g. on the command line using `echo -n 'password' | sha256sum`. Don't forget to shred your shell history after doing that.
Some shells don't log commands to history if they are prefixed with a space. Regarding authorization, Muonbase by now implements 
only simple HTTP basic authorization and there is no possibility to set user permissions on specific documents, 
meaning that all users can read and modify all documents in one collection. If you need user specific collections, scale 
horizontally and launch one new database server per user.

# API

## Routes
* POST /insert
* POST /erase
* POST /find
* GET /keys
* GET /values
* GET /image

### Insert

#### Request
```
POST /insert HTTP/1.1
authorization: Basic cm9vdDowMDAw
content-length: 1003
content-type: application/json

[{"guNloO9A":[true,0.561933,901188,"8NGbsNfc",null],"OrJxzNTq":{"RV6fLLMW":false,"tC3TF09H":0.358690,"zfKtUEbG":607057,"rOv9Tq5u":"lKKdJAFt","fsm9iOxx":null},"BiyEstkf":null,"c8EwQ9n9":"9IKxj6Qw","8G1CM6i9":639241,"wAmM4BsW":0.820225,"QwH0ksWj":true},{"fYzO9Yhz":{"yDIo1eVG":true,"F0P054A9":0.181838,"XFtqAxGv":986763,"uaBJRTal":"2KFMjDGa","rJmFQ0Kf":null},"N1yBKo1R":null,"iAE4GJOw":"jViv0Xy2","t6MnkZkA":364141,"hCnj2vpX":0.602900,"DuJSNbBv":[false,0.663713,287146,"voLwZrgz",null],"k1hYxghN":false},{"wOyqExt2":[false,0.924033,829322,"JJZIHgDT",null],"nZqI4xBq":{"I7oiOonw":false,"2oONVCI6":"KNA1MfiU","3Bxa3YxI":null,"hd3fC765":0.113570,"yMwH1Xpy":389680},"S6LcNwfV":null,"sV1JmMYm":"HaHmT8n0","4HQBiBNc":77599,"nSIv4pr2":0.369883,"8GtXcBJk":false},{"vwY9jRg9":[false,0.127580,521125,"agFetsyc",null],"xBYeX6MS":{"2g6rv0CL":false,"Le6kdwmh":0.755341,"zUCBsAZC":59639,"w0cTZh0S":"0zb86b3B","RzDAGnmL":null},"wb5UrJbR":null,"PTb9WVh3":"tprSwcCQ","KQQoyujm":403777,"Ty7aChNj":0.032886,"GxcMsJMA":true}]
```

#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 45
content-type: application/json
date: 20220114155834
server: muonbase/1

["yiIcR6RF","SUGVhqpD","wCynAboQ","WQW2wN1s"]
```

### Erase

#### Request
```
POST /erase HTTP/1.1
authorization: Basic cm9vdDowMDAw
content-length: 45
content-type: application/json

["yiIcR6RF","SUGVhqpD","wCynAboQ","WQW2wN1s"]
```

#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 45
content-type: application/json
date: 20220114155834
server: muonbase/1

["yiIcR6RF","SUGVhqpD","wCynAboQ","WQW2wN1s"]
```

### Find

#### Request
```
POST /find HTTP/1.1
authorization: Basic cm9vdDowMDAw
content-length: 104853
content-type: application/json

["04DUt7AN","04NCCAiC","04hCtVO5","050gz87F","05nuxJV1","060rL8qh","06DgimZJ","07rohPGa","08BEJKdS","09GlGW46","0ARfA0JA","0CIXsxF5","0D029FxG","0F4IMoRq","0FJv69S1","0H3BLalf","0HGXfexv","0J45THh4","0K5nSDWO","0MLjP1mW","0Meq8jWO","0NbxbX61","0NpW0uAq","0OOXf31b","0P1ExUcs","0QCaJFnt","0QHizo6E","0QnjPHH0","0QvKH3en","0R1U1wC6","0U4JtwmY","0VFgQJ9N","0Vh1km08","0ViAMRr9","0WQYsVZd","0XYt7Vrh","0XgMRpnQ","0YItWhmR","0YOnnH83","0Z06MCIh","0Z7RZSo9","0ZygF5nm","0a1DCX1m","0a6efeLX","0a6gkhy9","0aZuwKu6","0bPcn4vp","0beEm45p","0bwjzxy4","0c7hefIA","0cLLQZfZ","0cqX2LGR","0dEjT74Q","0iUnyIGj","0iZg0N6S","0jAdG4VG","0ja1hlt9","0kAn0hbw","0kN0E2y1","0kkLQ7CV","0krCH2cV","0krZSUir","0l0pMqbp","0lJczQtt","0lXaMpBE","0o6ta9mh","0p5MFxN4","0p7vpAML","0qhFHxOJ","0r4M1zzr","0sKwTwbU","0sq7Ht8e","0tj2dQKa","0vCBXxsJ","0vJ7A9Wy","0wbXkvhG","0xFS2uKR","0xOk3BXb","0xoE0pM2","0z5Q8pG9","0zhRXqOK","11x35Xgh","12PMAk5M","12iGbWIF","12zx7MOF","13wXYVnM","14CQqhJR","15rEOveW","16CzxSH6","16J26d1m","16Sw3GKn","171a5Dty","17Q30LDw",...
```

#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 2032984
content-type: application/json
date: 20220114155835
server: muonbase/1

[{"vpEtmw5b":false,"Pzi21R0c":0.567430,"VAVcaonJ":558932,"aXFpa1kF":"0qSBqeyJ","6fkCdRxb":{"5xrdWFgC":null,"b3FmfObr":824061,"hNFHhPve":0.166583,"xp2PPSe2":"qfwcMDQf","TIUvh2yD":false},"91wjBIim":null,"EakrhjKH":[true,0.239117,869812,"jZRMK7nc",null]},{"UW0z1zgo":false,"WQInIWHK":0.240893,"16zYHPhs":869889,"aDeUTiOA":"XYXp66rn","3ZNgyfOK":null,"5KJiLNtD":{"wnEqPApd":false,"uEdqDliF":0.146820,"685pIiYE":"xJsQd9Cx","SneZUuUj":860294,"sLzxPOn7":null},"RRAGk1gP":[false,0.952779,890675,"RI1gHSYD",null]},{"UlJcektq":false,"82WSCLq7":0.235730,"8J4st4dN":null,"EXtrcoYo":479940,"S5jJS32d":"xrpUsgmM","iEWueo2j":{"515ubRmD":true,"zluEN1GC":0.033444,"txB8vGPb":954753,"3YRe1zIC":"KDycqOfX","FzZgXXhE":null},"R1nL5bF4":[false,0.909517,14295,"Qg7EiQFj",null]},{"H36kaeWc":false,"2BWo3yzy":1005188,"iP6oSk5T":0.918809,"FoZtMxb0":"7NEes002","EHlVYpN4":null,"BeuFuH20":{"98SrriHR":null,"pIZL7Btr":"XU6wT7JZ","yu2gJYTi":662960,"kJdUgpiE":0.231272,"a48xtotL":true},"eZFkQnCU":[true,0.125975,781211,"GB0sN1pX",null]},{"8i17IApQ":true,"6...
```
### Keys

#### Request
```
GET /keys HTTP/1.1
authorization: Basic cm9vdDowMDAw
```

#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 104853
content-type: application/json
date: 20220114155828
server: muonbase/1

["04DUt7AN","04NCCAiC","04hCtVO5","050gz87F","05nuxJV1","060rL8qh","06DgimZJ","07rohPGa","08BEJKdS","09GlGW46","0ARfA0JA","0CIXsxF5","0D029FxG","0F4IMoRq","0FJv69S1","0H3BLalf","0HGXfexv","0J45THh4","0K5nSDWO","0MLjP1mW","0Meq8jWO","0NbxbX61","0NpW0uAq","0OOXf31b","0P1ExUcs","0QCaJFnt","0QHizo6E","0QnjPHH0","0QvKH3en","0R1U1wC6","0U4JtwmY","0VFgQJ9N","0Vh1km08","0ViAMRr9","0WQYsVZd","0XYt7Vrh","0XgMRpnQ","0YItWhmR","0YOnnH83","0Z06MCIh","0Z7RZSo9","0ZygF5nm","0a1DCX1m","0a6efeLX","0a6gkhy9","0aZuwKu6","0bPcn4vp","0beEm45p","0bwjzxy4","0c7hefIA","0cLLQZfZ","0cqX2LGR","0dEjT74Q","0iUnyIGj","0iZg0N6S","0jAdG4VG","0ja1hlt9","0kAn0hbw","0kN0E2y1","0kkLQ7CV","0krCH2cV","0krZSUir","0l0pMqbp","0lJczQtt","0lXaMpBE","0o6ta9mh","0p5MFxN4","0p7vpAML","0qhFHxOJ","0r4M1zzr","0sKwTwbU","0sq7Ht8e","0tj2dQKa","0vCBXxsJ","0vJ7A9Wy","0wbXkvhG","0xFS2uKR","0xOk3BXb","0xoE0pM2","0z5Q8pG9","0zhRXqOK","11x35Xgh","12PMAk5M","12iGbWIF","12zx7MOF","13wXYVnM","14CQqhJR","15rEOveW","16CzxSH6","16J26d1m","16Sw3GKn","171a5Dty","17Q30LDw",...
```

### Values

#### Request
```
GET /values HTTP/1.1
authorization: Basic cm9vdDowMDAw
```

#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 2386040
content-type: application/json
date: 20220114155830
server: muonbase/1

[{"vpEtmw5b":false,"Pzi21R0c":0.567430,"VAVcaonJ":558932,"aXFpa1kF":"0qSBqeyJ","6fkCdRxb":{"5xrdWFgC":null,"b3FmfObr":824061,"hNFHhPve":0.166583,"xp2PPSe2":"qfwcMDQf","TIUvh2yD":false},"91wjBIim":null,"EakrhjKH":[true,0.239117,869812,"jZRMK7nc",null]},{"UW0z1zgo":false,"WQInIWHK":0.240893,"16zYHPhs":869889,"aDeUTiOA":"XYXp66rn","3ZNgyfOK":null,"5KJiLNtD":{"wnEqPApd":false,"uEdqDliF":0.146820,"685pIiYE":"xJsQd9Cx","SneZUuUj":860294,"sLzxPOn7":null},"RRAGk1gP":[false,0.952779,890675,"RI1gHSYD",null]},{"UlJcektq":false,"82WSCLq7":0.235730,"8J4st4dN":null,"EXtrcoYo":479940,"S5jJS32d":"xrpUsgmM","iEWueo2j":{"515ubRmD":true,"zluEN1GC":0.033444,"txB8vGPb":954753,"3YRe1zIC":"KDycqOfX","FzZgXXhE":null},"R1nL5bF4":[false,0.909517,14295,"Qg7EiQFj",null]},{"H36kaeWc":false,"2BWo3yzy":1005188,"iP6oSk5T":0.918809,"FoZtMxb0":"7NEes002","EHlVYpN4":null,"BeuFuH20":{"98SrriHR":null,"pIZL7Btr":"XU6wT7JZ","yu2gJYTi":662960,"kJdUgpiE":0.231272,"a48xtotL":true},"eZFkQnCU":[true,0.125975,781211,"GB0sN1pX",null]},{"8i17IApQ":true,"6...
```

### Image

#### Request
```
GET /image HTTP/1.1
authorization: Basic cm9vdDowMDAw
```

#### Response
```
HTTP/1.1 200 OK
access-control-allow-methods: GET, POST
access-control-allow-origin: *
content-length: 2490892
content-type: application/json
date: 20220114155833
server: muonbase/1

{"zzr5LnsH":{"ncbUwsBQ":[false,0.265096,193415,"C3aQQq9L",null],"Vr9rB8SU":null,"TtjcoESY":"TJOu4tze","BdMPeHbn":{"QACpIKkd":null,"fr8cN7U4":"6epwVv6A","cHa0o7j9":197795,"2LwRv8bE":0.787538,"kWC23Hsc":true},"ryP6MUmU":769357,"fqzTlMKm":0.705919,"9YHE3L5T":false},"zzqau98i":{"c5bQscHb":{"n2kVFSgo":true,"4u5kfjRA":0.872484,"HkygXUsT":4699,"HrpFZZzc":"fEqYtimw","LP9Pwzjy":null},"JhTwN8zs":null,"5iJbIcmn":"2oMkBFlD","rOsyzNI4":444145,"yhbY7qvg":[false,0.584231,419467,"k9L85xyW",null],"FEoxZw84":0.468937,"mT0Xnvbr":false},"zzfetCp7":{"aRA48kWl":false,"3wzBxhMx":"nj02dxuc","k01ULZXC":0.630318,"xkiVNGVK":231285,"pTw5bfkn":null,"OCWNH9JX":{"8rbCQVFw":false,"SvEwWves":0.629154,"06Nt7tOF":73181,"GNdDZ4rF":"qZh2e4W7","QB5I3aUy":null},"1XfxXPKx":[false,0.574921,388153,"DcwFXjW6",null]},"zzQKc0J5":{"Fktc0mne":[false,0.614807,791088,"EY446uHW",null],"XbhaefQR":{"hciZceOp":true,"rajn9Vyx":0.675364,"0WhH3dCT":532056,"45Va45N5":null,"0f5x8n7k":"2imFMQTK"},"gtZz0NMd":null,"uqD6SV2R":0.969152,"v1CmX7g8":"RPW0Eh63","g3KBDOAk":33...
```

# Benchmark
Though implementing HTTP REST, Muonbase is highly performant as the benchmarks suggest. In the following example, four threads are simultaneously inserting and erasing random documents, where processing times are around one millisecond. Note that the processing time is measured as the complete HTTP request and response time (measured in a locally connected client) divided by the number of documents that are affected by the operation.
```
[3516|21.01.2022-23:17:18|info|source/test.cc:128] available service found on 127.0.0.1:8260
[3516|21.01.2022-23:17:18|info|source/test.cc:146] thread 0 started
[3516|21.01.2022-23:17:18|info|source/test.cc:146] thread 2 started
[3516|21.01.2022-23:17:18|info|source/test.cc:146] thread 3 started
[3516|21.01.2022-23:17:18|info|source/test.cc:146] thread 1 started
[3516|21.01.2022-23:17:29|info|source/test.cc:160] thread 0 fill db took 0.741290ms per insertion
[3516|21.01.2022-23:17:29|info|source/test.cc:160] thread 2 fill db took 0.747771ms per insertion
[3516|21.01.2022-23:17:29|info|source/test.cc:160] thread 3 fill db took 0.741381ms per insertion
[3516|21.01.2022-23:17:29|info|source/test.cc:160] thread 1 fill db took 0.745466ms per insertion
[3516|21.01.2022-23:17:39|info|source/test.cc:178] thread 0 cycle 1 took 0.710467ms per insertion
[3516|21.01.2022-23:17:39|info|source/test.cc:178] thread 2 cycle 1 took 0.720587ms per insertion
[3516|21.01.2022-23:17:40|info|source/test.cc:178] thread 3 cycle 1 took 0.716768ms per insertion
[3516|21.01.2022-23:17:40|info|source/test.cc:178] thread 1 cycle 1 took 0.727198ms per insertion
[3516|21.01.2022-23:17:49|info|source/test.cc:195] thread 0 cycle 1 took 0.607129ms per erasure
[3516|21.01.2022-23:17:54|info|source/test.cc:178] thread 0 cycle 2 took 0.379271ms per insertion
[3516|21.01.2022-23:17:58|info|source/test.cc:195] thread 1 cycle 1 took 1.227128ms per erasure
[3516|21.01.2022-23:17:58|info|source/test.cc:195] thread 2 cycle 1 took 1.239757ms per erasure
[3516|21.01.2022-23:17:59|info|source/test.cc:195] thread 3 cycle 1 took 1.277629ms per erasure
[3516|21.01.2022-23:18:07|info|source/test.cc:178] thread 1 cycle 2 took 0.625199ms per insertion
[3516|21.01.2022-23:18:07|info|source/test.cc:178] thread 2 cycle 2 took 0.626124ms per insertion
[3516|21.01.2022-23:18:08|info|source/test.cc:178] thread 3 cycle 2 took 0.608676ms per insertion
[3516|21.01.2022-23:18:11|info|source/test.cc:195] thread 0 cycle 2 took 1.094233ms per erasure
[3516|21.01.2022-23:18:21|info|source/test.cc:195] thread 1 cycle 2 took 0.928922ms per erasure
[3516|21.01.2022-23:18:21|info|source/test.cc:195] thread 2 cycle 2 took 0.916651ms per erasure
[3516|21.01.2022-23:18:21|info|source/test.cc:195] thread 3 cycle 2 took 0.896016ms per erasure
[3516|21.01.2022-23:18:21|info|source/test.cc:213] GET /keys
[3516|21.01.2022-23:18:22|info|source/test.cc:215] GET /values
[3516|21.01.2022-23:18:29|info|source/test.cc:217] GET /image
[3516|21.01.2022-23:18:38|info|source/test.cc:219] POST /insert
[3516|21.01.2022-23:18:38|info|source/test.cc:221] POST /erase
[3516|21.01.2022-23:18:38|info|source/test.cc:223] POST /find
[3516|21.01.2022-23:18:47|info|source/test.cc:229] all tests passed
```
