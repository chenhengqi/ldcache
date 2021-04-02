# Howto

## ld.so.cache.old

`ld.so.cache.old` is obtained from a container using `centos` image.

```
$ docker run -d --rm --name ldcacheold centos sleep 100
$ docker cp ldcache:/etc/ld.so.cache .
```

## ld.so.cache

`ld.so.cache` is obtained from a container using `ubuntu` image.

```
$ docker run -d --rm --name ldcache ubuntu sleep 100
$ docker cp ldcache:/etc/ld.so.cache .
```
