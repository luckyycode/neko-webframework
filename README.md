# Neko server / Mvc

Neko server with Mvc framework (work in progress).

![alt text](https://d.radikal.ru/d34/1806/1b/a9e011b101ec.png)

### Prerequisites

To build this you'll need

```
Neko Framework (no sub-dependencies!) - will be available soon in my repositories
OpenSSL (optional!)
```

### Installing

You'll need premake5. Launch the following commands:

```
1. cd ../project_dir/
2. premake5 --file=Neko.lua (see lua file for customizable flags, e.g. USE_OPENSSL)
3. Open with desired ide
```
Sample project is also available in this repository, check out SampleModule folder.

## Roadmap

* HTTP/2 support - *wip* 
* GnuTLS support instead of OpenSSL (I will keep these together)
* More samples
* Mvc - database
* Mvc - user 
* Mvc - session

## License

soon
