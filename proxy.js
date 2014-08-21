// https://en.wikipedia.org/wiki/Proxy_auto-config
// http://www-archive.mozilla.org/docs/netlib/pac.html

function FindProxyForURL(url, host) {
    isp = "PROXY ip_address:port; DIRECT";
    tor = "SOCKS 127.0.0.1:9050";
    bit = "SOCKS 127.0.0.1:8878";
    if (shExpMatch(host, "*.onion")) return tor;
    if (shExpMatch(host, "*.bitweb")) return bit;
    return "DIRECT";
}
