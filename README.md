# Overview

![alt text](https://c.radikal.ru/c25/1807/13/e500422fd6a7.png)
![alt text](https://img.shields.io/badge/repo%20status-active-blue.svg)

Neko server / framework. Work in progress, not ready for production in any way yet.

Fast and efficient server with higher level mvc code for developing web applications.

See Wiki page for more info.

| Directory                  | Description       
| --------------------- |:-----------------------------------------------------------------:|
| /Nova/                     | contains higher level logic (e.g. controllers, routing)
| /Skylar/                  | Skylar server, contains lower level logic for processing requests      
| /ContentTypes/        | supported content data parsers for content-type      
| /SampleModule/     | sample project 


### Prerequisites

To build this you'll need

```
Neko Framework (no sub-dependencies!) - will be available soon in my repositories
OpenSSL (optional)
```

### Installing

You'll need premake5. Launch the following commands:

```
1. cd ../project_dir/
2. premake5 --file=Neko.lua (see lua file for customizable flags, e.g. use-openssl)
3. Open with desired ide
```
Sample project is also available in this repository, check out SampleModule folder.

## Roadmap

There are still a lot of things to do, this framework is not yet ready for production in any way.

See Issues for more info, but the most important and global now are the following features:

* HTTP/2 support - *wip* 
* GnuTLS support instead of OpenSSL (I will keep these together)
* More samples
* Nova - database

## About

## License

soon
![alt text](https://d.radikal.ru/d34/1806/1b/a9e011b101ec.png)
