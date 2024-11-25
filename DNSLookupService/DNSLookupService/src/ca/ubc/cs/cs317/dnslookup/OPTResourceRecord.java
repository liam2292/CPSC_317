package ca.ubc.cs.cs317.dnslookup;

import java.util.Arrays;
import java.util.Objects;

public class OPTResourceRecord implements ResourceRecord {
    /* Name is implicitly 0 (ROOT) */
    private final int payloadSize;
    private final int extendedRCodeAndFlags;
    private final byte[] rData;
    private final DNSQuestion question;

    public OPTResourceRecord(int payloadSize, int extendedRCodeAndFlags, byte[] rData, DNSQuestion question) {
        this.payloadSize = payloadSize;
        this.extendedRCodeAndFlags = extendedRCodeAndFlags;
        this.rData = rData;
        this.question = new DNSQuestion("", question.getRecordType(), question.getRecordClass());
    }

    @Override
    public RecordType getRecordType() {
        return RecordType.OPT;
    }

    @Override
    public int getRecordClassCode() {
        return payloadSize;
    }

    public int getPayloadSize() {
        return payloadSize;
    }

    public int getExtendedRCodeAndFlags() {
        return extendedRCodeAndFlags;
    }

    public byte[] getrData() {
        return rData;
    }

    @Override
    public DNSQuestion getQuestion() {
        return question;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        OPTResourceRecord that = (OPTResourceRecord) o;
        return payloadSize == that.payloadSize && extendedRCodeAndFlags == that.extendedRCodeAndFlags && Arrays.equals(rData, that.rData) && Objects.equals(question, that.question);
    }

    @Override
    public int hashCode() {
        int result = Objects.hash(payloadSize, extendedRCodeAndFlags, question);
        result = 31 * result + Arrays.hashCode(rData);
        return result;
    }

    @Override
    public String toString() {
        return String.format("[OPT: Payload %dB]", payloadSize);
    }
}
