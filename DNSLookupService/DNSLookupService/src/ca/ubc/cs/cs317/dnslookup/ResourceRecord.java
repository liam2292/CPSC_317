package ca.ubc.cs.cs317.dnslookup;

public interface ResourceRecord {
    RecordType getRecordType();
    int getRecordClassCode();
    DNSQuestion getQuestion();
}
