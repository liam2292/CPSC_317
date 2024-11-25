package ca.ubc.cs.cs317.dnslookup;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class DNSCacheTest {
    @Test
    public void testConstructor() {
        DNSCache cache = DNSCache.getInstance();
    }
    @Test
    public void testGetBestNameServer() {
        DNSCache cache = DNSCache.getInstance();
        cache.reset();
        DNSQuestion question = new DNSQuestion("norm.cs.ubc.ca", RecordType.NS, RecordClass.IN);
        List<CommonResourceRecord> nslist = cache.getBestNameservers(question);
        List<CommonResourceRecord> alist = cache.filterByKnownIPAddress(nslist);
        assertEquals(13, nslist.size());
        assertEquals(13, alist.size());
    }
    @Test
    public void testGetBestNameServerLower() {
        DNSCache cache = DNSCache.getInstance();
        cache.reset();
        DNSQuestion question = DNSCache.NSQuestion("norm.cs.ubc.ca");
        cache.addResult(new CommonResourceRecord(DNSCache.NSQuestion("cs.ubc.ca"), 3600, "dns.cs.ubc.ca"));
        List<CommonResourceRecord> nslist = cache.getBestNameservers(question);
        List<CommonResourceRecord> alist = cache.filterByKnownIPAddress(nslist);
        assertEquals(1, nslist.size());
        assertEquals(0, alist.size());
        cache.addResult(new CommonResourceRecord(DNSCache.AQuestion("ns.cs.ubc.ca"), 3600, DNSCache.stringToInetAddress("142.103.10.10")));
    }
}
