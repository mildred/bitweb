BitWeb
======

This is a cntinuation of the [P2PWeb](//github.com/mildred/p2pweb) project,
using the bittorrent technology. In a few words, this is a proxy software that
you can plug your browser with, and that will let you access torrents as if they
were websites.

For example, when you point yout browser to
`http://dbbda49df0642ebc45f142059e60552a70cc3c5b.bitweb/index.html`, the proxy
will download the torrent that has the info_hash
`dbbda49df0642ebc45f142059e60552a70cc3c5b`, will look at the file `index.html`
and will serve it over http to your browser. This is the same technology as the
magnet links for torrents.

The purpose of this is to have:

- be able to put a website online without having to buy for hardware. This
  reduce costs to create a website and do not force website authors to use
  advertisement to pay for these costs. The end user benefits from it as he is
  not tracked by the advertisement companies.

- promote static websites that don't rely on server software. Here, we just
  can't. If you need a server-specific behaviour, you are creating a web
  application. You could distribute the frontend using bitweb and let the user
  choose the backend.

- decentralize completely the web. Don't separate the client from the server.
  Having a distributed system where anyone can add a server is great, but is not
  enough. People will always want to buy the servers from someone else because
  this is more convenient. And you have big centralized services all over again.

- bring back privacy on the web. With no server to keep track of you requests,
  it is much more difficult to track your activity.

- make it much more difficult for  central authority to block websites.

Description and installation
============================

The software is both a SOCKS4A proxy (that can reply to HTTP requests for
`.bitweb` domains) and a bittorrent client.

In order to have your browser redirect to this proxy, create a `proxy.js` file
containing:

    function FindProxyForURL(url, host) {
        isp = "PROXY ip_address:port; DIRECT";
        tor = "SOCKS 127.0.0.1:9050";
        bit = "SOCKS 127.0.0.1:8878";
        if (shExpMatch(host, "*.onion")) return tor;
        if (shExpMatch(host, "*.bitweb")) return bit;
        return "DIRECT";
    }

Put the url of this file (using the `file:` protocol) in the proxy settings of
your browser for the autoconfiguration URL.

Usage
=====

To start the proxy server on the default port (8878), run:

    bitweb -D

To create a torrent suitable for websites, use the command line:

    bitweb -Ct mywebsite.torrent -k privatekey.p8.der /path/to/my/website

You can add metadata to your pages:

    bitweb -Ut mywebsite.torrent -k privatekey.p8.der -p index.html -H "content-type:text/html; charset=utf-8"

You can visualize your torrent:

    bitweb -St mywebsite.torrent

You can seed your torrent:

    info_hash=$(bitweb -St mywebsite.torrent | grep 'info hash' | cut -d' ' -f3)
    torrent_name="$(bitweb -St mywebsite.torrent | grep 'info hash' | cut -d' ' -f3-)"
    
    cp mywebsite.torrent ~/.cache/bitweb/$info_hash.torrent
    mkdir ~/.cache/bitweb/$info_hash
    cp -R /path/to/my/website "~/.cache/bitweb/$info_hash/$torrent_name"

(and restart your server, `HUP` signal doesn't work yet)

Further work, Bittorrent extensions
===================================

Padding files (implemented)
-------------

In order to be able to share pieces between different versions of a torrent, add
padding files in the torrent to make sure that every legitimate file is aligned
with a piece boundary. Use the libtorrent approach.

Torrent signature (implemented)
-----------------

A info dict can contain a RSA public key and a signature that validates the
owner of the corresponding private key authored the info dict. The signature is
computed using the bencoding of the info dict (with the signature field
removed).

For a request to a specific info_hash, the rule is to trust the key included in
that info_hash when requesting future revisions.

TODO: Use [BEP 35, signing](http://bittorrent.org/beps/bep_0035.html) instead?
The problem is that the signature is outsie the info dict and may not be
transmitted when initializing through magnet links. Or completely forget about
this part as it will be taken care of by the torrent revisions

File metadata (implemented)
-------------

Add HTTP header information for each file in a torrent (the content-type for
example).

Torrent versions
----------------

A torrent can reference previous versions. When referencing a previous version,
a torrent must also reference recursively all the previous versions. See below
for the implementation details.

TODO: maybe we might want to make this more general than bitweb. Avoid using the
string "bitweb" when implementing that feature. maybe it could be proposed as a
BEP.

Backlinks
---------

A torrent can reference other torrents (or a specific file in other torrents).
A link is composed of a source (torent info hash and filename), a relation type
(uri that describes what relation the source has to the destination) and a
destination (info hash of a torrent and a filename).

Forward links should be stored in the torrent itself and backlinks should be
available in the DHT.

Encryption
----------

torrents (info dict and content itself) should be able to be encrypted using a
password (that can be embedded in HTML links). like freenet, this would allow
anyone to share space and bandwidth for websites you can't know anything about.
Notes would advertise that they are willing to share disk space or bandwidth and
would download automatically such torrents.

Reference
=========

- [SOCKS4](http://www.openssh.com/txt/socks4.protocol)
- [SOCKS4A](http://www.openssh.com/txt/socks4a.protocol)
- [libtorrent](http://www.rasterbar.com/products/libtorrent/)
- [BEP 35: signing](http://bittorrent.org/beps/bep_0035.html)
- [BEP 38: share files between torrents](http://bittorrent.org/beps/bep_0038.html)
- [XDG basedir specification](http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html)

Hacking
=======

Small tasks:

- Auto detect file mime type on torrent creation: Have a
  `--autodetect-content-type` when creating a torrent file.

- Honor [XDG basedir specification](http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html)

- Command line to seed a site:
  `bitweb -A|--add -t TORRENTFILE --copy|--hardlink DIR`
  would copy (`--reflink` if possible) or hardlink `DIR` to
  `~/.local/share/bitweb`.

- HTTP server: add mode to listen to a HTTP port (default would be 80). Use the
  HTTP/1.1 Host request header to detect the info_hash of the requested site, or
  the name of a torrent that is in ~/.local/share/bitweb (not in cache). Default
  to a specified info hash if no torrent found. Add ability to specify a
  correspondance table between a domain name and a info hash.

- use libtorrent share mode

- port code to libtorrent 1.x (I am using 0.x because ArchLinux package is not
  yet up to date).

Handle torrent versions using [DHT store extension](http://www.rasterbar.com/products/libtorrent/dht_store.html)
-------------------------------------------------

Implementation note: `session::dht_get_item()` and `session::dht_put_item()`

In the info dict, add a "bitweb" entry containing a dict with:

- `public key`: ed25519 public key (32 bytes string)
- `id`: any string that, together with the public key) will uniquely identify
   the website.
- `seq`: the revision of the website. Starts at 1. (integer)
- `parents`: a list containing the revisions of the website. The first item
   being the first revision (having `seq` equal to 1). For each revision the 20
   bytes of the info hash is stored as a string. The list must contain `seq - 1`
   items. No less, no more.
- `parent signatures`: a list containing the signatures for the parents.

New revisions are made by incrementing the `seq` number and adding the parent
hash info to the `parents` list.

Revisions are advertised using the [DHT store extension](http://www.rasterbar.com/products/libtorrent/dht_store.html).
A mutable item is stored with:

- `k`: the `public key` value defined above
- `salt`: a 27 bytes string. The first 7 bytes being `"bitweb:"` and the last 20
   bytes being the sha1 hash of the `id` value defined above.
- `seq`: the `seq` value defined above
- `v`: the info hash of the torrent
- `auto update`: a boolean true (integer 1) or false (integer 0). If not
   specified, true is assumed.

The `cas` field is not present, the `id`, `sig` and `token` fields are computed
as defined in the DHT store extension.

To allow anyone to republish a version of the torrent, the torrent should
contain outside of the info dict the signature required to republish the value.
It cannot be included inside the info dict because the signature depends on the
info hash value.

TODO: this signature should be obtained by nodes when they use magnet links to
obtain the torrent. The client can check that the signature obtained is valid by
checking it against the public key and the site id stored inside the info hash.

*Other idea: instead of looking up websites by the info hash, look them up
against `sha1(info_hash + signature)` and have this information stored as an
immutable message through the DHT store extension. The problem with that is that
it adds an indirection layer and will slow down downloading a torrent for the
first time, and initial HTTP connection.*

When obtained, and if a torrent file is written to disk, the signature should be
included in a `bitweb signature` key at the same level of the `info` dict. Every
bitweb client that share a website should regularly republish the latest
signature.

URL conventions:

- `http://INFOHASH=.bitweb`: the website at the revision of `INFOHASH` exactly
- `http://INFOHASH=N.bitweb`: the website at the Nth revision as specified in
  the `INFOHASH` torrent
- `http://INFOHASH+.bitweb`: the website at the latest possible revision
- `http://INFOHASH+N.bitweb`: the website at the Nth revision as specified in
  the latest possible revision of `INFOHASH`
- `http://INFOHASH.bitweb`: either the `INFOHASH=.bitweb` or `INFOHASH+.bitweb`
  depending on the `auto update` value specified in the `bitweb` dict.

TODO: be notified in real time when a site is updated and do not require
pooling frequently for updates.

Hardening code
--------------

Each socks client (source IP and port) in a separate process (in case the
process crash). Avoid too much processes (one process per  connection) and avoid
crashing other users or the main daemon.

Have a separate process for the torrent node as well.

Simple related tasks includes:

- Add exception handlers
- Add SIGSEGV handler that would print debug info and exec to a new instance.

Code issues
===========

Crypto++
--------

Can't load DER encoded key? Check it is in PKCS#8 format

    openssl pkcs8 -in privkey.pem -out privkey.p8 -topk8 -nocrypt -outform der

See: http://crypto-users.996303.n3.nabble.com/Load-Private-RSA-key-td2851.html
And: http://www.cryptopp.com/fom-serve/cache/62.html
And: http://stackoverflow.com/questions/9869261/crypto-load-rsa-pkcs1-private-key-from-memory
And: http://www.cryptopp.com/wiki/Keys_and_Formats

See: http://stackoverflow.com/questions/25441918/crypto-cant-der-encode-and-ber-decode-rsa-public-key-ber-decode-error
