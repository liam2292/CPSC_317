package ca.ubc.cs.cs317.dnslookup;

import java.io.Serializable;
import java.util.Objects;

/** DNS nodes can be used to specify an individual DNS query or the key to a specific result.
 * Each node represents a fully-qualified domain name (represented by hostName) and a record
 * type. Two nodes with the same host name and type are considered equal.
 */
public class DNSQuestion implements Comparable<DNSQuestion>, Serializable {

    private final String hostName;
    private final RecordType type;
    private final RecordClass recordClass;

    public DNSQuestion(String hostName, RecordType type, RecordClass recordClass) {
        this.hostName = hostName;
        this.type = type;
        this.recordClass = recordClass;
    }

    public String getHostName() {
        return hostName;
    }

    public RecordType getRecordType() {
        return type;
    }

    public RecordClass getRecordClass() {
        return recordClass;
    }

    @Override
    public String toString() {
        return (hostName.isEmpty() ? "<root>" : hostName) + " (" + type + ")";
    }

    @Override
    public int compareTo(DNSQuestion o) {
        if (!hostName.equalsIgnoreCase(o.hostName))
            return hostName.compareToIgnoreCase(o.hostName);
        if (!hostName.equals(o.hostName))
            return hostName.compareTo(o.hostName);
        if (!recordClass.equals(o.recordClass))
            return recordClass.compareTo(o.recordClass);
        return type.compareTo(o.type);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        DNSQuestion that = (DNSQuestion) o;
        return hostName.equals(that.hostName) && type == that.type && recordClass == that.recordClass;
    }

    @Override
    public int hashCode() {
        return Objects.hash(hostName, type, recordClass);
    }
}
