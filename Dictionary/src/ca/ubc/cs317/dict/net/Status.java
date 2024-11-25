package ca.ubc.cs317.dict.net;

import java.io.BufferedReader;
import java.io.IOException;

/**
 * Created by Jonatan on 2017-09-09.
 */
public class Status {

    public static final int PRELIMINARY_REPLY = 1;
    public static final int COMPLETION_REPLY = 2;
    public static final int INTERMEDIATE_REPLY = 3;
    public static final int TRANSIENT_NEGATIVE_REPLY = 4;
    public static final int PERMANENT_NEGATIVE_REPLY = 5;

    private int statusCode;
    private String details;

    private Status(String line) throws DictConnectionException {
        if (line == null)
            throw new DictConnectionException("Status line expected");
        String[] components = line.split(" ", 2);
        if (components.length < 2)
            throw new DictConnectionException("Invalid status line");
        try {
            this.statusCode = Integer.parseInt(components[0]);
            if (this.statusCode < 100 || this.statusCode > 599)
                throw new DictConnectionException("Invalid status code received: " + this.statusCode);
        } catch (NumberFormatException ex) {
            throw new DictConnectionException("Status code number expected (" + line + ")", ex);
        }
        this.details = components[1];
    }

    public static Status readStatus(BufferedReader input) throws DictConnectionException {
        try {
            return new Status(input.readLine());
        } catch (IOException ex) {
            throw new DictConnectionException();
        }
    }

    public int getStatusCode() {
        return statusCode;
    }

    public int getStatusType() {
        return statusCode / 100;
    }

    public String getDetails() {
        return details;
    }

    public boolean isNegativeReply() {
        return getStatusType() == TRANSIENT_NEGATIVE_REPLY ||
                getStatusType() == PERMANENT_NEGATIVE_REPLY;
    }
}
