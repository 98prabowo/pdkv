## pdkv (patria dev key-value storage)

This is redis-like trivial key-value storage. Below are the key ingredients:

```
- Binary search tree algorithm (not self-balanced)
- DJB2 hash function
```

Installation:

```
- clone the repo
- run 'make'
- ./pdkv
- connect to port 31337 and you're set :)
```

Available commands:

```
- GET <key>
- SET <key> <value>
- DEL / DELETE <key>
```

This software lacks of the must-implemented features below:

```
- multithread
- dynamic buffer management for request / response
- thread-safe memory allocation
```
