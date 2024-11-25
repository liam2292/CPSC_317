package ca.ubc.cs.cs317.dnslookup;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.IntStream;

public class DNSMessage {
    public static final int MAX_DNS_MESSAGE_LENGTH = 512;
    public static final int MAX_EDNS_MESSAGE_LENGTH = 1280;
    public static final int IDOffset = 0;
    public static final int CMDOffset = 2;
    public static final int QDCountOffset = 4;
    public static final int ANCountOffset = 6;
    public static final int NSCountOffset = 8;
    public static final int ARCountOffset = 10;
    public static final int DataOffset = 12;
    public static final int QRMask = 0b1000000000000000;
    public static final int OpcodeMask = 0b0111100000000000;
    public static final int AAMask = 0b0000010000000000;
    public static final int RDMask = 0b0000000100000000;
    public static final int RAMask = 0b0000000010000000;
    public static final int RcodeMask = 0b0000000000001111;
    public static final int TCMask = 0b0000001000000000;
    public static final int ShortMask = 0xffff;
    public static final int PtrMask = 0xc0;
    public static final int PtrShortMask = 0xc000;
    public static final int PtrHOBMask = 0x3f;
    public static final int PtrHOBShift = 8;
    public static final int OpcodeShift = 11;
    public static final int ByteMask = 0xff;
    public static final int QUERY = 0;
    private final Map<String, Integer> nameToPosition = new HashMap<>();
    private final Map<Integer, String> positionToName = new HashMap<>();
    private final ByteBuffer buffer;

    /**
     * Initializes an empty DNSMessage with the given id.
     *
     * @param id The id of the message.
     */
    public DNSMessage(short id) {
        this.buffer = ByteBuffer.allocate(MAX_DNS_MESSAGE_LENGTH);
        short zero = 0;
        buffer.putShort(IDOffset, id);
        buffer.putShort(CMDOffset, zero);
        buffer.putShort(QDCountOffset, zero);
        buffer.putShort(ANCountOffset, zero);
        buffer.putShort(NSCountOffset, zero);
        buffer.putShort(ARCountOffset, zero);
        buffer.position(DataOffset);
    }

    /**
     * Initializes a DNSMessage with the first length bytes of the given byte array.
     *
     * @param recvd  The byte array containing the received message
     * @param length The length of the data in the array
     */
    public DNSMessage(byte[] recvd, int length) {
        buffer = ByteBuffer.wrap(recvd, 0, length);
        buffer.position(DataOffset);
    }

    /**
     * Getters and setters for the various fixed size and fixed location fields of a DNSMessage
     */
    public int getID() {
        return buffer.getShort(IDOffset) & ShortMask;
    }

    public void setID(int id) {
        buffer.putShort(IDOffset, (short) (id & ShortMask));
    }

    public boolean getQR() {
        int cmd = buffer.getShort(CMDOffset);
        return (cmd & QRMask) == QRMask;
    }

    public void setQR(boolean qr) {
        int cmd = buffer.getShort(CMDOffset);
        cmd = qr ? (cmd | QRMask) : (cmd & ~QRMask);
        buffer.putShort(CMDOffset, (short) cmd);
    }

    public boolean getAA() {
        int cmd = buffer.getShort(CMDOffset);
        return (cmd & AAMask) == AAMask;
    }

    public void setAA(boolean aa) {
        int cmd = buffer.getShort(CMDOffset);
        cmd = aa ? (cmd | AAMask) : (cmd & ~AAMask);
        buffer.putShort(CMDOffset, (short) cmd);
    }

    public int getOpcode() {
        int cmd = buffer.getShort(CMDOffset);
        return (cmd & OpcodeMask) >> OpcodeShift;
    }

    public void setOpcode(int opcode) {
        int cmd = buffer.getShort(CMDOffset);
        cmd = (cmd & ~OpcodeMask) | (opcode << OpcodeShift);
        buffer.putShort(CMDOffset, (short) cmd);
    }

    public boolean getTC() {
        int cmd = buffer.getShort(CMDOffset);
        return (cmd & TCMask) == TCMask;
    }

    public void setTC(boolean tc) {
        int cmd = buffer.getShort(CMDOffset);
        cmd = tc ? (cmd | TCMask) : (cmd & ~TCMask);
        buffer.putShort(CMDOffset, (short) cmd);
    }

    public boolean getRD() {
        int cmd = buffer.getShort(CMDOffset);
        return (cmd & RDMask) == RDMask;
    }

    public void setRD(boolean rd) {
        int cmd = buffer.getShort(CMDOffset);
        cmd = rd ? (cmd | RDMask) : (cmd & ~RDMask);
        buffer.putShort(CMDOffset, (short) cmd);
    }

    public boolean getRA() {
        int cmd = buffer.getShort(CMDOffset);
        return (cmd & RAMask) == RAMask;
    }

    public void setRA(boolean ra) {
        int cmd = buffer.getShort(CMDOffset);
        cmd = ra ? (cmd | RAMask) : (cmd & ~RAMask);
        buffer.putShort(CMDOffset, (short) cmd);
    }

    public int getRcode() {
        int cmd = buffer.getShort(CMDOffset);
        return (cmd & RcodeMask);
    }

    public void setRcode(int rcode) {
        int cmd = buffer.getShort(CMDOffset);
        cmd = (cmd & ~RcodeMask) | (rcode & RcodeMask);
        buffer.putShort(CMDOffset, (short) cmd);
    }

    public int getQDCount() {
        return buffer.getShort(QDCountOffset) & ShortMask;
    }

    public void setQDCount(int count) {
        buffer.putShort(QDCountOffset, (short) count);
    }

    public int getANCount() {
        return buffer.getShort(ANCountOffset) & ShortMask;
    }

    public int getNSCount() {
        return buffer.getShort(NSCountOffset) & ShortMask;
    }

    public int getARCount() {
        return buffer.getShort(ARCountOffset) & ShortMask;
    }

    public void setANCount(int count) {
        buffer.putShort(ANCountOffset, (short) count);
    }

    public void setNSCount(int count) {
        buffer.putShort(NSCountOffset, (short) count);
    }

    public void setARCount(int count) {
        buffer.putShort(ARCountOffset, (short) count);
    }

    /**
     * Return the name at the current position() of the buffer.  This method is provided for you,
     * but you should ensure that you understand what it does and how it does it.
     * <p>
     * The trick is to keep track of all the positions in the message that contain names, since
     * they can be the target of a pointer.  We do this by storing the mapping of position to
     * name in the positionToName map.
     *
     * @return The decoded name
     */
    public String getName() {
        // Remember the starting position for updating the name cache
        int start = buffer.position();
        int len = buffer.get() & ByteMask;
        if (len == 0) return "";
        if ((len & PtrMask) == PtrMask) {  // This is a pointer
            int pointer = ((len & PtrHOBMask) << PtrHOBShift) | (buffer.get() & ByteMask);
            String suffix = positionToName.get(pointer);
            assert suffix != null;
            positionToName.put(start, suffix);
            return suffix;
        }
        byte[] bytes = new byte[len];
        buffer.get(bytes, 0, len);
        String label = new String(bytes, StandardCharsets.UTF_8);
        String suffix = getName();
        String answer = suffix.isEmpty() ? label : label + "." + suffix;
        positionToName.put(start, answer);
        return answer;
    }

    /**
     * The standard toString method that displays everything in a message.
     *
     * @return The string representation of the message
     */
    public String toString() {
        // Remember the current position of the buffer so we can put it back
        // Since toString() can be called by the debugger, we want to be careful to not change
        // the position in the buffer.  We remember what it was and put it back when we are done.
        int end = buffer.position();
        try {
            StringBuilder sb = new StringBuilder();
            sb.append("ID: ").append(getID()).append(' ');
            sb.append("QR: ").append(getQR()).append(' ');
            sb.append("OP: ").append(getOpcode()).append(' ');
            sb.append("AA: ").append(getAA()).append('\n');
            sb.append("TC: ").append(getTC()).append(' ');
            sb.append("RD: ").append(getRD()).append(' ');
            sb.append("RA: ").append(getRA()).append(' ');
            sb.append("RCODE: ").append(getRcode()).append(' ')
                    .append(dnsErrorMessage(getRcode())).append('\n');
            sb.append("QDCount: ").append(getQDCount()).append(' ');
            sb.append("ANCount: ").append(getANCount()).append(' ');
            sb.append("NSCount: ").append(getNSCount()).append(' ');
            sb.append("ARCount: ").append(getARCount()).append('\n');
            buffer.position(DataOffset);
            showQuestions(getQDCount(), sb);
            showRRs("Authoritative", getANCount(), sb);
            showRRs("Name servers", getNSCount(), sb);
            showRRs("Additional", getARCount(), sb);
            return sb.toString();
        } catch (Exception e) {
            e.printStackTrace();
            return "toString failed on DNSMessage";
        } finally {
            buffer.position(end);
        }
    }

    /**
     * Add the text representation of all the questions (there are nq of them) to the StringBuilder sb.
     *
     * @param nq Number of questions
     * @param sb Collects the string representations
     */
    private void showQuestions(int nq, StringBuilder sb) {
        sb.append("Question [").append(nq).append("]\n");
        for (int i = 0; i < nq; i++) {
            DNSQuestion question = getQuestion();
            sb.append('[').append(i).append(']').append(' ').append(question).append('\n');
        }
    }

    /**
     * Add the text representation of all the resource records (there are nrrs of them) to the StringBuilder sb.
     *
     * @param kind Label used to kind of resource record (which section are we looking at)
     * @param nrrs Number of resource records
     * @param sb   Collects the string representations
     */
    private void showRRs(String kind, int nrrs, StringBuilder sb) {
        sb.append(kind).append(" [").append(nrrs).append("]\n");
        for (int i = 0; i < nrrs; i++) {
            ResourceRecord rr = getRR();
            sb.append('[').append(i).append(']').append(' ').append(rr).append('\n');
        }
    }

    /**
     * Decode and return the question that appears next in the message.  The current position in the
     * buffer indicates where the question starts.
     *
     * @return The decoded question
     */
    public DNSQuestion getQuestion() {
        String hostname = getName();
        short type = buffer.getShort();
        short klass = buffer.getShort();
        RecordType rtype = RecordType.getByCode(type);
        RecordClass rklass = RecordClass.getByCode(klass);
        return new DNSQuestion(hostname, rtype, rklass);
    }

    /**
     * Decode and return the resource record that appears next in the message.  The current
     * position in the buffer indicates where the resource record starts.
     *
     * @return The decoded resource record
     */
    public ResourceRecord getRR() {
        String cname;
        String mxname;
        String nsname;
        byte[] rdata;
        ResourceRecord rr;
        String owner = getName();
        short type = buffer.getShort();
        short klass = buffer.getShort();
        int ttl = buffer.getInt();
        RecordType rtype = RecordType.getByCode(type);
        RecordClass rklass = RecordClass.getByCode(klass);
        DNSQuestion question = new DNSQuestion(owner, rtype, rklass);
        int rdatalen = buffer.getShort() & ShortMask;
        int startpos = buffer.position();
        if (rtype == RecordType.A && rklass == RecordClass.IN) {
            rdata = new byte[4];
            buffer.get(rdata, 0, 4);
            InetAddress address;
            try {
                address = InetAddress.getByAddress(rdata);
                rr = new CommonResourceRecord(question, ttl, address);
            } catch (UnknownHostException e) {
                rr = null;
            }
        } else if (rtype == RecordType.AAAA && rklass == RecordClass.IN) {
            rdata = new byte[16];
            buffer.get(rdata, 0, 16);
            InetAddress address;
            try {
                address = InetAddress.getByAddress(rdata);
                rr = new CommonResourceRecord(question, ttl, address);
            } catch (UnknownHostException e) {
                rr = null;
            }
        } else if (rtype == RecordType.CNAME) {
            cname = getName();
            rr = new CommonResourceRecord(question, ttl, cname);
        } else if (rtype == RecordType.MX) {
            //noinspection unused We ignore mx preference fields
            int pref = buffer.getShort() & ShortMask;
            mxname = getName();
            rr = new CommonResourceRecord(question, ttl, mxname);
        } else if (rtype == RecordType.NS) {
            nsname = getName();
            rr = new CommonResourceRecord(question, ttl, nsname);
        } else if (rtype == RecordType.SOA) {
            rdata = new byte[rdatalen];
            buffer.get(rdata, 0, rdatalen);
            String hex = byteArrayToHexString(rdata);
            rr = new CommonResourceRecord(question, ttl, hex);
        } else if (rtype == RecordType.OPT) {
            assert owner.isEmpty();
            rdata = new byte[rdatalen];
            buffer.get(rdata, 0, rdatalen);
            rr = new OPTResourceRecord(/*payload size*/ klass,
                    /*extended RCODE and flags*/ ttl, rdata, question);
        } else {
            rdata = new byte[rdatalen];
            buffer.get(rdata, 0, rdatalen);
            String hex = byteArrayToHexString(rdata);
            rr = new CommonResourceRecord(question, ttl, hex);
        }
        int endpos = buffer.position();
        assert endpos - startpos == rdatalen;
        return rr;
    }

    /**
     * Helper function that returns a hex string representation of a byte array. May be used to represent the result of
     * records that are returned by a server but are not supported by the application (e.g., SOA records).
     *
     * @param data a byte array containing the record data.
     * @return A string containing the hex value of every byte in the data.
     */
    public static String byteArrayToHexString(byte[] data) {
        return IntStream.range(0, data.length).mapToObj(i -> String.format("%02x", data[i])).reduce("", String::concat);
    }

    /**
     * Helper function that returns a byte array from a hex string representation. May be used to represent the result of
     * records that are returned by a server but are not supported by the application (e.g., SOA records).
     *
     * @param hexString a string containing the hex value of every byte in the data.
     * @return data a byte array containing the record data.
     */
    public static byte[] hexStringtoByteArray(String hexString) {
        byte[] bytes = new byte[hexString.length() / 2];
        for (int i = 0; i < bytes.length; i++) {
            String s = hexString.substring(i * 2, i * 2 + 2);
            bytes[i] = (byte) Integer.parseInt(s, 16);
        }
        return bytes;
    }

    /**
     * Add an encoded name to the message. It is added at the current position and uses compression
     * as much as possible.  Compression is accomplished by remembering the position of every added
     * label.
     *
     * @param name The name to be added
     */
    public void addName(String name) {
        String label;
        while (name.length() > 0) {
            Integer offset = nameToPosition.get(name);
            if (offset != null) {
                int pointer = offset;
                pointer |= PtrShortMask;
                buffer.putShort((short) pointer);
                return;
            } else {
                nameToPosition.put(name, buffer.position());
                int dot = name.indexOf('.');
                label = (dot > 0) ? name.substring(0, dot) : name;
                buffer.put((byte) label.length());
                for (int j = 0; j < label.length(); j++) {
                    buffer.put((byte) label.charAt(j));
                }
                name = (dot > 0) ? name.substring(dot + 1) : "";
            }
        }
        buffer.put((byte) 0);
    }

    /**
     * Add an encoded question to the message at the current position.
     *
     * @param question The question to be added
     */
    public void addQuestion(DNSQuestion question) {
        addName(question.getHostName());
        addQType(question.getRecordType());
        addQClass(question.getRecordClass());
        setQDCount(getQDCount() + 1);
    }

    /**
     * Add an encoded resource record to the message at the current position.
     * The record is added to the additional records section.
     *
     * @param rr The resource record to be added
     */
    public void addResourceRecord(CommonResourceRecord rr) {
        addResourceRecord(rr, "additional");
    }

    /**
     * Add an encoded resource record to the message at the current position.
     *
     * @param rr      The resource record to be added
     * @param section Indicates the section to which the resource record is added
     */
    public void addResourceRecord(ResourceRecord rr, String section) {
        addName(rr.getQuestion().getHostName());
        addQType(rr.getRecordType());
        if (rr.getRecordType() == RecordType.OPT) {
            buffer.putShort((short)rr.getRecordClassCode());
        } else {
            addQClass(rr.getQuestion().getRecordClass());
        }
        String cname;
        String mxname;
        String nsname;
        byte[] rdata;

        if (rr.getRecordType() == RecordType.OPT) {
            buffer.putInt(((OPTResourceRecord)rr).getExtendedRCodeAndFlags());
        } else {
            buffer.putInt((int) ((CommonResourceRecord)rr).getRemainingTTL());
        }

        int startpos = buffer.position();
        buffer.putShort((short)0);
        if (rr.getRecordType() == RecordType.A && rr.getQuestion().getRecordClass() == RecordClass.IN) {
            assert rr instanceof CommonResourceRecord;
            InetAddress address = ((CommonResourceRecord)rr).getInetResult();
            rdata = address.getAddress();
            if (rdata.length != 4) throw new RuntimeException("Resource record length incorrect");
            buffer.put(rdata, 0, rdata.length);
        } else if (rr.getRecordType() == RecordType.AAAA && rr.getQuestion().getRecordClass() == RecordClass.IN) {
            assert rr instanceof CommonResourceRecord;
            InetAddress address = ((CommonResourceRecord)rr).getInetResult();
            rdata = address.getAddress();
            if (rdata.length != 16) throw new RuntimeException("Resource record length incorrect");
            buffer.put(rdata, 0, rdata.length);
        } else if (rr.getRecordType() == RecordType.CNAME) {
            assert rr instanceof CommonResourceRecord;
            cname = ((CommonResourceRecord)rr).getTextResult();
            addName(cname);
        } else if (rr.getRecordType() == RecordType.MX) {
            assert rr instanceof CommonResourceRecord;
            short pref = (short)0;
            buffer.putShort(pref);
            mxname = ((CommonResourceRecord)rr).getTextResult();
            addName(mxname);
        } else if (rr.getRecordType() == RecordType.NS) {
            assert rr instanceof CommonResourceRecord;
            nsname = ((CommonResourceRecord)rr).getTextResult();
            addName(nsname);
        } else if (rr.getRecordType() == RecordType.OPT) {
            assert rr instanceof OPTResourceRecord;
            buffer.put(((OPTResourceRecord)rr).getrData());
        } else {
            assert rr instanceof CommonResourceRecord;
            rdata = hexStringtoByteArray(((CommonResourceRecord)rr).getTextResult());
            buffer.put(rdata);
        }
        int endpos = buffer.position();
        int rdatalen = endpos - startpos - 2;
        buffer.putShort(startpos, (short)rdatalen);
        switch (section) {
            case "answer":
                setANCount(getANCount() + 1);
                break;
            case "nameserver":
                setNSCount(getNSCount() + 1);
                break;
            case "additional":
                setARCount(getARCount() + 1);
                break;

        }
    }

    /**
     * Add an encoded type to the message at the current position.
     *
     * @param recordType The type to be added
     */
    private void addQType(RecordType recordType) {
        short qtype = (short) recordType.getCode();
        buffer.putShort(qtype);
    }

    /**
     * Add an encoded class to the message at the current position.
     *
     * @param recordClass The class to be added
     */
    private void addQClass(RecordClass recordClass) {
        short qclass = (short) recordClass.getCode();
        buffer.putShort(qclass);
    }

    /**
     * Return a byte array that contains all the data comprising this message.  The length of the
     * array will be exactly the same as the current position in the buffer.
     *
     * @return A byte array containing this message's data
     */
    public byte[] getUsed() {
        int length = buffer.position();
        byte[] res = new byte[length];
        buffer.position(0);
        buffer.get(res, 0, length);
        return res;
    }

    /**
     * Returns a string representation of a DNS error code.
     *
     * @param error The error code received from the server.
     * @return A string representation of the error code.
     */
    public static String dnsErrorMessage(int error) {
        final String[] errors = new String[]{
                "No error", // 0
                "Format error", // 1
                "Server failure", // 2
                "Name error (name does not exist)", // 3
                "Not implemented (parameters not supported)", // 4
                "Refused" // 5
        };
        if (error >= 0 && error < errors.length)
            return errors[error];
        return "Invalid error message";
    }
}
