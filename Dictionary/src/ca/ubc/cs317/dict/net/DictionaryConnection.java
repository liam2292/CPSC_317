package ca.ubc.cs317.dict.net;

import ca.ubc.cs317.dict.model.Database;
import ca.ubc.cs317.dict.model.Definition;
import ca.ubc.cs317.dict.model.MatchingStrategy;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.*;

import static ca.ubc.cs317.dict.net.DictStringParser.splitAtoms;
import static ca.ubc.cs317.dict.net.Status.readStatus;

/**
 * Created by Jonatan on 2017-09-09.
 */
public class DictionaryConnection {

    private static final int DEFAULT_PORT = 2628;

    //private variables
    private final Socket socket;
    private final BufferedReader in;
    private final PrintWriter out;

    private Map<String, Database> databases = new LinkedHashMap<String, Database>();

    /** Establishes a new connection with a DICT server using an explicit host and port number, and handles initial
     * welcome messages.
     *
     * @param host Name of the host where the DICT server is running
     * @param port Port number used by the DICT server
     * @throws DictConnectionException If the host does not exist, the connection can't be established, or the messages
     * don't match their expected value.
     */
    public DictionaryConnection(String host, int port) throws DictConnectionException {
        // TODO Replace this with code that creates the requested connection
        try {
            socket = new Socket(host, port);
            System.out.println("connected");
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);
            Status status = readStatus(in);

            if (status.getStatusCode() != 220) {
                throw new DictConnectionException(status.getDetails());
            }
        } catch(Exception e) {
            throw new DictConnectionException(e);
        }
    }

    /** Establishes a new connection with a DICT server using an explicit host, with the default DICT port number, and
     * handles initial welcome messages.
     *
     * @param host Name of the host where the DICT server is running
     * @throws DictConnectionException If the host does not exist, the connection can't be established, or the messages
     * don't match their expected value.
     */
    public DictionaryConnection(String host) throws DictConnectionException {
        this(host, DEFAULT_PORT);
    }

    /** Sends the final QUIT message and closes the connection with the server. This function ignores any exception that
     * may happen while sending the message, receiving its reply, or closing the connection.
     *
     */
    public synchronized void close() {
        // TODO Add your code here
        out.println("QUIT");
        try {
            in.close();
            out.close();
            socket.close();
            System.out.println("Disconnected...");
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    /** Requests and retrieves all definitions for a specific word.
     *
     * @param word The word whose definition is to be retrieved.
     * @param database The database to be used to retrieve the definition. A special database may be specified,
     *                 indicating either that all regular databases should be used (database name '*'), or that only
     *                 definitions in the first database that has a definition for the word should be used
     *                 (database '!').
     * @return A collection of Definition objects containing all definitions returned by the server.
     * @throws DictConnectionException If the connection was interrupted or the messages don't match their expected value.
     */

    public synchronized Collection<Definition> getDefinitions(String word, Database database) throws DictConnectionException {
        Collection<Definition> set = new ArrayList<>();

        // TODO Add your code here
        String message = "DEFINE " + database.getName() + " \"" + word + "\"";
        out.println(message);
        Status response = readStatus(in);
        if (response.getStatusCode() == 150) {
            String[] firstResponse = splitAtoms(response.getDetails());
            int numDef = Integer.parseInt(firstResponse[0]);
            for (int i = 0; i < numDef; i++) {
                response = readStatus(in);
                String[] match = splitAtoms(response.getDetails());
                if (response.getStatusCode() != 151) {
                    throw new DictConnectionException();
                }
                Definition df;
                String line;
                try {
                    df = new Definition(word, match[1]);
                    line = in.readLine();
                    while (!line.equals(".")) {
                        df.appendDefinition(line);
                        line = in.readLine();
                    }
                } catch (IOException e) {
                    throw new DictConnectionException();
                }
                set.add(df);
                }
                response = readStatus(in);
                if (response.getStatusCode() != 250) {
                    throw new DictConnectionException();
                }
            } else {
                return set;
            }
        return set;
    }

    /** Requests and retrieves a list of matches for a specific word pattern.
     *
     * @param word     The word whose definition is to be retrieved.
     * @param strategy The strategy to be used to retrieve the list of matches (e.g., prefix, exact).
     * @param database The database to be used to retrieve the definition. A special database may be specified,
     *                 indicating either that all regular databases should be used (database name '*'), or that only
     *                 matches in the first database that has a match for the word should be used (database '!').
     * @return A set of word matches returned by the server.
     * @throws DictConnectionException If the connection was interrupted or the messages don't match their expected value.
     */
    public synchronized Set<String> getMatchList(String word, MatchingStrategy strategy, Database database) throws DictConnectionException {
        Set<String> set = new LinkedHashSet<>();
        // TODO Add your code here

        String message = "MATCH " + database.getName() + " " + strategy.getName() + " " + "\"" + word + "\"";
        out.println(message);
        Status response = readStatus(in);
        if (response.getStatusCode() == 552){
            return set;
        }
        if (response.getStatusCode() == 152) {
                String line;
                try {
                    line = in.readLine();
                    while (!line.equals(".")) {
                        String[] input = splitAtoms(line);
                        set.add(input[1]);
                        line = in.readLine();
                    }
                } catch (IOException e) {
                    throw new DictConnectionException();
                }
                response = readStatus(in);
                if (response.getStatusCode() != 250) {
                    throw new DictConnectionException();
                }
        } else {
            return set;
        }
        return set;
    }

    /** Requests and retrieves a map of database name to an equivalent database object for all valid databases used in the server.
     *
     * @return A map of Database objects supported by the server.
     * @throws DictConnectionException If the connection was interrupted or the messages don't match their expected value.
     */
    public synchronized Map<String, Database> getDatabaseList() throws DictConnectionException {
        Map<String, Database> databaseMap = new LinkedHashMap<String, Database>();
        // TODO Add your code here
        out.println("SHOW DB");
        Status response = readStatus(in);
        if (response.getStatusCode() == 110) {
            String line;
            try {
                line = in.readLine();
                while (!line.equals(".")) {
                    String[] input = splitAtoms(line);
                    Database db = new Database(input[0], input[1]);
                    databaseMap.put(db.getName(), db);
                    line = in.readLine();
                }
            }catch (IOException e) {
                throw new DictConnectionException();
            }
            response = readStatus(in);
            if (response.getStatusCode() != 250) {
                throw new DictConnectionException();
            }
        } else {
            return databaseMap;
        }
        return databaseMap;
    }

    /** Requests and retrieves a list of all valid matching strategies supported by the server.
     *
     * @return A set of MatchingStrategy objects supported by the server.
     * @throws DictConnectionException If the connection was interrupted or the messages don't match their expected value.
     */
    public synchronized Set<MatchingStrategy> getStrategyList() throws DictConnectionException {
        Set<MatchingStrategy> set = new LinkedHashSet<>();

        // TODO Add your code here
        out.println("SHOW STRAT");
        Status response = readStatus(in);
        if (response.getStatusCode() == 111) {
                String line;
                try {
                    line = in.readLine();
                    while (!line.equals(".")) {
                        String[] input = splitAtoms(line);
                        set.add(new MatchingStrategy(input[0], input[1]));
                        line = in.readLine();
                    }
                } catch (IOException e) {
                    throw new DictConnectionException();
                }
                response = readStatus(in);
                if (response.getStatusCode() != 250) {
                    throw new DictConnectionException();
                }
        } else {
           return set;
        }
        return set;
    }

    /** Requests and retrieves detailed information about the currently selected database.
     *
     * @return A string containing the information returned by the server in response to a "SHOW INFO <db>" command.
     * @throws DictConnectionException If the connection was interrupted or the messages don't match their expected value.
     */
    public synchronized String getDatabaseInfo(Database d) throws DictConnectionException {

        // TODO Add your code here
        out.println("SHOW INFO " + d);
        Status response = readStatus(in);
        String info = "";
        if (response.getStatusCode() == 112) {
            String line;

            try {
                line = in.readLine();
                while (!line.equals(".")) {
                    info += line + "\n";
                    line = in.readLine();
                }
            } catch (IOException e) {
                throw new DictConnectionException();
            }

            response = readStatus(in);

            if (response.getStatusCode() != 250) {
                throw new DictConnectionException();
            } else {
                return info;
            }
        }
        return info;
    }

}


