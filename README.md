update 2020: It's slowly in wip state for now due to my another projects.
update 2023: it is being heavily developed and used under some other project, may release production-proof version soon, i hope

# Overview

![alt text](https://c.radikal.ru/c25/1807/13/e500422fd6a7.png)

![alt text](https://d25lcipzij17d.cloudfront.net/badge.svg?id=gh&type=6&v=dev&x2=0)
![alt text](https://img.shields.io/badge/repo%20status-active-blue.svg)
![alt text](https://img.shields.io/github/last-commit/luckyycode/neko-webframework.svg)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/1153bf48ef35457597c2e05af41a5dd7)](https://app.codacy.com/app/luckyycode/neko-webframework?utm_source=github.com&utm_medium=referral&utm_content=luckyycode/neko-webframework&utm_campaign=badger)
[![Build Status](https://travis-ci.org/luckyycode/neko-webframework.svg?branch=master)](https://travis-ci.org/luckyycode/neko-webframework)

Neko server / web framework. Work in progress, not ready for production in any way yet.

Fast and efficient server with higher level mvc code for developing web applications.

# See the Wiki page [here](https://github.com/luckyycode/neko-webframework/wiki) for more info.

Structure:

| Directory                  | Description       
| --------------------- |:-----------------------------------------------------------------:|
| /Nova/                     | Contains higher level logic (e.g. controllers, routing)
| /Skylar/                  | Skylar server, contains lower level logic for processing requests      
| /ContentTypes/        | Supported content Data parsers for content-type      
| /SampleModule/     | Sample project 


### Prerequisites

To build this you'll need

```
- Neko Framework (no sub-dependencies!) - will be available soon in my repositories
- OpenSSL (optional)
```

### Installing

You'll need premake5. Launch the following commands:

```
1. cd ../project_dir/
2. premake5 --file=Neko.lua (see lua file for customizable flags, e.g. use-openssl) <project target>
3. Open with desired ide/make
```
Sample project is also available in this repository, check out SampleModule folder.

## Roadmap

There are still a lot of things to do, this framework is not yet ready for production in any way.

See Issues for more info, but the most important and global now are the following features:

* Setup Travis CI (currently failing due to empty settings)
* HTTP/2 support - *wip* 
* GnuTLS support instead of OpenSSL (I will keep these together)
* More samples
* Nova - database

## About

## License

soon
![alt text](https://d.radikal.ru/d34/1806/1b/a9e011b101ec.png)
