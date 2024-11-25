package ca.ubc.cs317.dict.net;

/**
 * Created by Jonatan on 2017-09-10.
 */
public class DictConnectionException extends Exception {

    public DictConnectionException() {
    }

    public DictConnectionException(Throwable cause) {
        super(cause);
    }

    public DictConnectionException(String message) {
        super(message);
    }

    public DictConnectionException(String message, Throwable cause) {
        super(message, cause);
    }
}
