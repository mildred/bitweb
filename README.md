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

File metadata (implemented)
-------------

Add HTTP header information for each file in a torrent (the content-type for
example).

Torrent versions
----------------

A torrent can reference previous versions. When referencing a previous version,
a torrent must also reference recursively all the previous versions.

The DHT must store for each torrent all the newer versions. For each version,
the DHT must also contain the public key (or hash of the public key) and the
list of parent versions. With this information, a client knows which is the
latest version (the greatest number of revisions with the same key).

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

Old Things (for personal reference)
===================================

Torrent file spec
-----------------

- Use signing spec at http://bittorrent.org/beps/bep_0035.html
- Use http://bittorrent.org/beps/bep_0038.html
    - include parents in similar keys
    - collection key should contain the public key

    torrent = dict {
        info
        announce
        announce-list
        creation date
        comment
        created by
    }

    torrent info = dict {
        piece length
        pieces
        name
        length [single file]
        files [multiple files]
        pubkey [bitweb]
        last version [bitweb]
        signature [bitweb]
    }

    torrent info "piece length" = integer
    torrent info pieces = string(20*x)
    torrent info name = string
    torrent info length = integer
    torrent info files = list dict {
        length
        path
        headers [bitweb]
    }
    torrent info pubkey = string
    torrent info "last version" = list string(20)
    torrent info "last version" = string as if the signature was absent

    torrent info files path = list string
    torrent info files length = integer
    torrent info files headers = dict

Note that info_hash = sha1(bencode(torrent info))

Make sure that identical files get identical pieces, through different version, even though they are moved.

Key format is DER (as understood by QSslKey, that is the binary information contained in the PEM format in a base64 form)

Command Line
------------

start daemon:

    bitweb -D|--daemon [-p PORT]

create torent file

    bitweb -C|--create -t TORRENTFILE [-p|--parent INFOHASH] DIR

show torrent file

    bitweb -S|--show -t TORRENTFILE [-f SUBFILE [-h HEADER]]

update torrent file

    bitweb -U|--update -t TORRENTFILE -f SUBFILE -h HEADER:VALUE...

SOCKS Interface
---------------

Access the INFOHASH website at this exact revision:

    http://INFOHASH=.bitweb

Access the INFOHASH website at the latest revision:

    http://INFOHASH.bitweb
    http://INFOHASH+.bitweb

The latest revision of an info hash is:

- a torrent that have the current info hash in its list of parents
- a torrent that is signed by the same key
- the torrent that have the most revions between the current infohash and the end of the parent list
- if multiple torrent mathes, take a torrent with less parents in its parent list

A torrent is advertised using its infohash and the inforhashes of the parent list.
A torrent advertisement contains its infohash and the infohash of all its
parent. For each infohash, the author identity is stored as well (hash of the
public key).

Crypto++
--------

Can't load DER encoded key? Check it is in PKCS#8 format

    openssl pkcs8 -in privkey.pem -out privkey.p8 -topk8 -nocrypt -outform der

See: http://crypto-users.996303.n3.nabble.com/Load-Private-RSA-key-td2851.html
And: http://www.cryptopp.com/fom-serve/cache/62.html
And: http://stackoverflow.com/questions/9869261/crypto-load-rsa-pkcs1-private-key-from-memory
And: http://www.cryptopp.com/wiki/Keys_and_Formats
