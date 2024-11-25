package ca.ubc.cs.cs317.dnslookup;

import java.io.Serializable;
import java.net.InetAddress;
import java.util.Date;
import java.util.Objects;

/**
 * A resource record corresponds to each individual result returned by a DNS response. It links a DNS question (host
 * name, type and class) to either an IP address (e.g., for A or AAAA records) or a textual response (e.g., for CNAME or
 * NS records). An expiration time is also specified, and computed based on the TTL provided when the record is
 * created.
 */
public class CommonResourceRecord implements Serializable, ResourceRecord {

    private final DNSQuestion question;
    private Date expirationTime;
    private final String textResult;
    private InetAddress inetResult;

    /**
     * Creates a new resource record based on a string result, without an InetAddress.
     *
     * @param question Question object containing the host name (FQDN), type and class associated to this record.
     * @param ttl      Number of seconds to keep this record in cache.
     * @param result   The string representation associated to the record's result. Its meaning depends on the type, but
     *                 for CNAME, NS and MX it represents the FQDN of the host associated to this record.
     */
    public CommonResourceRecord(DNSQuestion question, int ttl, String result) {
        this.question = question;
        this.expirationTime = new Date(System.currentTimeMillis() + ((long) ttl * 1000));
        this.textResult = result;
        this.inetResult = null;
    }

    /**
     * Creates a new resource record based on an InetAddress result (typically an A or AAAA record). The string
     * representation is also saved based on the getHostAddress method of InetAddress.
     *
     * @param question Question object containing the host name (FQDN), type and class associated to this record.
     * @param ttl      Number of seconds to keep this record in cache.
     * @param result   The InetAddress object associated to the record's result.
     */
    public CommonResourceRecord(DNSQuestion question, int ttl, InetAddress result) {
        this(question, ttl, result.getHostAddress());
        this.inetResult = result;
    }

    public DNSQuestion getQuestion() {
        return question;
    }

    public RecordType getRecordType() {
        return question.getRecordType();
    }

    public int getRecordClassCode() {
        return question.getRecordClass().getCode();
    }

    /**
     * The remaining TTL for this record, in seconds. It is rounded up, based on the remaining time until this record
     * expires. The TTL returned by this method will only match the TTL obtained from the DNS server in the first second
     * from the time this record was created.
     *
     * @return The number of seconds, rounded up, until this record expires.
     */
    public long getRemainingTTL() {
        return (expirationTime.getTime() - System.currentTimeMillis() + 999) / 1000;
    }

    /**
     * Returns true if this record has expired, and false otherwise. An expired record should not be maintained in
     * cache, instead a new record should be retrieved from an appropriate nameserver.
     *
     * @return true if this record has expired, and false otherwise.
     */
    public boolean isExpired() {
        return !expirationTime.after(new Date());
    }

    /**
     * Updates the current record with updated information from a new record. This will update the expiration time if
     * the new record contains a longer expiration time.
     *
     * @param record Another resource record with potentially new information.
     */
    public void update(CommonResourceRecord record) {
        if (this.expirationTime.before(record.expirationTime))
            this.expirationTime = record.expirationTime;
    }

    public String getTextResult() {
        return textResult;
    }

    public InetAddress getInetResult() {
        return inetResult;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        CommonResourceRecord that = (CommonResourceRecord) o;
        return question.equals(that.question) &&
                textResult.equals(that.textResult) &&
                Objects.equals(inetResult, that.inetResult);
    }

    @Override
    public int hashCode() {
        return Objects.hash(question, textResult, inetResult);
    }

    @Override
    public String toString() {
        return "[" + question + " -> " + textResult + "]";
    }
}
