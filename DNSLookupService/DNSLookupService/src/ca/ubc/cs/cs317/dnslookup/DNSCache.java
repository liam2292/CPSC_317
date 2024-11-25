package ca.ubc.cs.cs317.dnslookup;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.*;
import java.util.function.BiConsumer;

/**
 * This class handles a cache of DNS results. It is based on a map that links questions to a set of resource
 * records (results). Cached results are only maintained for the duration of the TTL (time-to-live) returned by the
 * server.  Expired entries are removed each time the cache is accessed.
 */
public class DNSCache {

    public static final DNSQuestion rootQuestion = NSQuestion("");
    private static final String[][] rootServers = {
            { "a.root-servers.net", "198.41.0.4" },
            { "b.root-servers.net", "199.9.14.201" },
            { "c.root-servers.net", "192.33.4.12" },
            { "d.root-servers.net", "199.7.91.13" },
            { "e.root-servers.net", "192.203.230.10" },
            { "f.root-servers.net", "192.5.5.241" },
            { "g.root-servers.net", "192.112.36.4" },
            { "h.root-servers.net", "198.97.190.53" },
            { "i.root-servers.net", "192.36.148.17" },
            { "j.root-servers.net", "192.58.128.30" },
            { "k.root-servers.net", "193.0.14.129" },
            { "l.root-servers.net", "199.7.83.42" },
            { "m.root-servers.net", "202.12.27.33" }
    };
    private static final Set<CommonResourceRecord> rootNameServersSet = new HashSet<>();
    private static final DNSCache instance = new DNSCache();
    private final Map<DNSQuestion, Set<CommonResourceRecord>> cachedResults = new HashMap<>();

    private DNSCache() {
        reset();
    }

    /**
     * Singleton retrieval method. Only one instance of the DNS cache can be created. This method returns the single DNS
     * cache instance.
     *
     * @return Instance of a DNS cache.
     */
    public static DNSCache getInstance() {
        return instance;
    }

    /**
     * Resets the cache to its initial value, containing only root nameservers.
     */
    public void reset() {
        reset(rootServers);
    }

    public void reset(String[] ... nameservers) {
        cachedResults.clear();
        this.cachedResults.put(rootQuestion, rootNameServersSet);
        rootNameServersSet.clear();
        for (String[] nameserver : nameservers) {
            String name = nameserver[0];
            String ipAddress = nameserver[1];
            InetAddress address = stringToInetAddress(ipAddress);
            CommonResourceRecord rr = new CommonResourceRecord(rootQuestion, Integer.MAX_VALUE, name);
            rootNameServersSet.add(rr);
            DNSQuestion q = AQuestion(name);
            rr = new CommonResourceRecord(AQuestion(name), Integer.MAX_VALUE, address);
            Set<CommonResourceRecord> value = new HashSet<>();
            value.add(rr);
            this.cachedResults.put(q, value);
        }
    }

    /**
     * Returns a set of resource records already cached for a particular query. If no results are cached for the
     * specified query, returns an empty set. Expired results are removed from the cache before being returned. This
     * method does not perform the query itself, it only returns previously cached results. Results are returned in
     * random order.
     *
     * @param question     DNS query (host name/type/class) for the results to be obtained.
     * @return A potentially empty set of resources associated to the query.
     */
    public List<CommonResourceRecord> getCachedResults(DNSQuestion question) {
        List<CommonResourceRecord> ans = new ArrayList<>();
        pruneExpired();
        Set<CommonResourceRecord> results = cachedResults.get(question);
        if (results != null) {
            ans.addAll(results);
        }

        // Include in the results any records we have for the canonical name (if any)
        results = cachedResults.get(new DNSQuestion(question.getHostName(), RecordType.CNAME, question.getRecordClass()));
        if (results != null) {
            for (CommonResourceRecord r : results) {
                DNSQuestion aquestion = new DNSQuestion(r.getTextResult(), RecordType.A, r.getQuestion().getRecordClass());
                Set<CommonResourceRecord> arecords = cachedResults.get(aquestion);
                if (!ans.contains(r)) ans.add(r);
                if (arecords != null) {
                    for (CommonResourceRecord rr : arecords) {
                        if (!ans.contains(rr)) ans.add(rr);
                    }
                }
            }
        }

        Collections.shuffle(ans);
        return ans;
    }

    /**
     * Returns a set of resource records for the best nameservers to query for a given question.
     * Will never return an empty set since the root nameservers are always candidates if no better
     * nameserver can be found.
     *
     * @param question      DNS query (host name/type/class) for which a nameserver is desired.
     * @return              A non-empty set of NS resources that are the best ones to answer this query.
     */
    public List<CommonResourceRecord> getBestNameservers(DNSQuestion question) {
        List<CommonResourceRecord> returningList = new ArrayList<>();
        pruneExpired();
        for (String suffix : new Suffixes(question.getHostName())) {
            DNSQuestion nsquestion = new DNSQuestion(suffix, RecordType.NS, RecordClass.IN);
            List<CommonResourceRecord> nslist = getCachedResults(nsquestion);
            if (!nslist.isEmpty()) {
                returningList.addAll(nslist);
                break;
            }
        }
        Collections.shuffle(returningList);
        return returningList;
    }

    /**
     * Returns a collection of A resource records for the subset of the provided nameservers
     * for which IPV4 addresses are known.
     * The result collection may be empty if none of the provided nameservers have a known IP address.
     *
     * @param servers     Collection of nameservers (NS records)
     * @return A collection of A records providing the IP addresses of those servers for whom
     * the IP address is known.
     */
    public List<CommonResourceRecord> filterByKnownIPAddress(Collection<CommonResourceRecord> servers) {
        List<CommonResourceRecord> returningList = new ArrayList<>();
        pruneExpired();
        for (CommonResourceRecord server : servers) {
            String host = server.getTextResult();
            DNSQuestion question = AQuestion(host);
            List<CommonResourceRecord> alist = getCachedResults(question);
            returningList.addAll(alist);
        }
        Collections.shuffle(returningList);
        return returningList;
    }

    /**
     * Adds a specific resource record to the DNS cache. If the cache already has an equivalent resource record, the
     * existing record is updated if the new one expires after the existing record.
     *
     * @param record Resource record, possibly obtained from a DNS server, containing the result of a DNS query.
     */
    public void addResult(CommonResourceRecord record) {

        if (record.isExpired()) return;

        Set<CommonResourceRecord> results = cachedResults.computeIfAbsent(record.getQuestion(), q -> new HashSet<>());

        // Find a record for the same question containing the same result
        CommonResourceRecord oldRecord = results.stream().filter(record::equals).findFirst().orElse(null);
        if (oldRecord == null)
            results.add(record);
        else
            oldRecord.update(record);
    }

    /**
     * Perform a specific action for each query and its set of cached records. This action can be specified using a
     * lambda expression or method name. Expired records are removed before the action is performed.
     *
     * @param consumer Action to be performed for each query and set of records.
     */
    public void forEachQuestion(BiConsumer<DNSQuestion, Collection<CommonResourceRecord>> consumer) {
        cachedResults.forEach(consumer);
    }

    /**
     * Perform a specific action for each query and individual record. This action can be specified using a lambda
     * expression or method name. Expired records are removed before the action is performed.
     *
     * @param consumer Action to be performed for each query and record.
     */
    @SuppressWarnings("unused")
    public void forEachRecord(BiConsumer<DNSQuestion, CommonResourceRecord> consumer) {
        forEachQuestion((question, records) -> records.forEach(record -> consumer.accept(question, record)));
    }

    /**
     * Remove all expired resource records from the cache.  If this results in the set of resource records
     * associated with a question becoming empty, also remove the question from the cache.
     */
    private void pruneExpired() {
        Set<DNSQuestion> toRemove = new HashSet<>();
        cachedResults.forEach((question, records) -> {
            records.removeIf(CommonResourceRecord::isExpired);
            if (records.isEmpty())
                toRemove.add(question);
        });
        for (DNSQuestion question : toRemove) {
            cachedResults.remove(question);
        }
    }

    /**
     * A utility class to return all the suffixes of a given DNS name.
     * If given www.cs.ubc.ca, the provided iterator will return:
     *   "cs.ubc.ca"
     *   "ubc.ca"
     *   "ca"
     *   ""  (the name of the root)
     */
    private static class Suffixes implements Iterable<String> {
        private final String string;

        public Suffixes(String string) {
            this.string = string;
        }
        private static class SuffixesIterator implements Iterator<String> {
            private String string;
            private boolean first = true;

            public SuffixesIterator(String string) {
                this.string = string;
            }

            @Override
            public boolean hasNext() {
                return !string.isEmpty();
            }

            @Override
            public String next() {
                if (first) {
                    first = false;
                    return string;
                }
                int firstdot = string.indexOf('.');
                if (firstdot >= 0) {
                    string = string.substring(firstdot + 1);
                } else {
                    string = "";
                }
                return string;
            }
        }
        @Override
        public Iterator<String> iterator() {
            return new SuffixesIterator(string);
        }
    }

    /**
     * Return a DNSQuestion for the given host name, with type A
     * @param name  The desired host name
     * @return      The desired DNSQuestion
     */
    public static DNSQuestion AQuestion(String name) {
        return new DNSQuestion(name, RecordType.A, RecordClass.IN);
    }

    /**
     * Return a DNSQuestion for the given name, with type NS
     * @param name  The desired name
     * @return      The desired DNSQuestion
     */
    public static DNSQuestion NSQuestion(String name) {
        return new DNSQuestion(name, RecordType.NS, RecordClass.IN);
    }

    /**
     * Return the InetAddress encoded in the given String.  The possible UnknownHostException
     * should never be thrown because this method must only be used to convert a dotted-decimal
     * String representation of an IP address to the actual address.
     * @param address   A string containing the dotted-decimal representation of an address
     *                  E.g., 142.103.6.6
     * @return          The InetAddress corresponding to address
     */
    public static InetAddress stringToInetAddress(String address) {
        try {
            return InetAddress.getByName(address);
        } catch (UnknownHostException e) {
            throw new RuntimeException("Can't map " + address + " to an InetAddress");
        }
    }
}
