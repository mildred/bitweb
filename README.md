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

