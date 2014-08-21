The server software would:

- Use C++2011 and boost
- Accept [SOCKS4A](http://www.openssh.com/txt/socks4a.protocol)
  ([SOCKS4](http://www.openssh.com/txt/socks4.protocol)) connections
- Accept HTTP connection on top of SOCKS4A. HTTP Protocol implementation could
  be provided by  [pion](https://github.com/splunk/pion) although there could be
  alternatives as referenced in [this thread](http://stackoverflow.com/questions/175507/c-c-web-server-library)
- Transform the domain name xxx.bitweb to a magnet link (xxx being the
  info-hash). Feed it to [libtorrent](http://www.rasterbar.com/products/libtorrent/).
- Download as fast as possible the metadata and the requested file. Download
  other files in background. make sure that the file being requested is
  downloaded in sequential order.
  This may be useful: http://www.rasterbar.com/products/libtorrent/streaming.html
- If the torrent is already downloading, make sure that the file requested is
  downloaded as fast as possible in sequential order.
- Serve the downloaded file as it is being downloaded. Do not wait for the
  torrent download to complete.

This would make a good demo. Improvement includes extnsions to bittorrent:

- store metadata with files (HTTP headers such as content type)
- store a public key with the torrent and sign the torrent
- either augment the torrent (if possible) or make a torrent reference a
  previous torrent. This would make torrent revisions.
- advertise torrent revisions on the DHT and download latest revision of a
  torrent by default. Allow mode that download specific revision.
- advertise metadata on DHT (backlinks).

Torrent file spec
=================

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
============

start daemon:

    bitweb -D|--daemon [-p PORT]

create torent file

    bitweb -C|--create -t TORRENTFILE [-p|--parent INFOHASH] DIR

show torrent file

    bitweb -S|--show -t TORRENTFILE [-f SUBFILE [-h HEADER]]

update torrent file

    bitweb -U|--update -t TORRENTFILE -f SUBFILE -h HEADER:VALUE...

SOCKS Interface
===============

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
========

Can't load DER encoded key? Check it is in PKCS#8 format

    openssl pkcs8 -in privkey.pem -out privkey.p8 -topk8 -nocrypt -outform der

See: http://crypto-users.996303.n3.nabble.com/Load-Private-RSA-key-td2851.html
And: http://www.cryptopp.com/fom-serve/cache/62.html
And: http://stackoverflow.com/questions/9869261/crypto-load-rsa-pkcs1-private-key-from-memory
And: http://www.cryptopp.com/wiki/Keys_and_Formats
