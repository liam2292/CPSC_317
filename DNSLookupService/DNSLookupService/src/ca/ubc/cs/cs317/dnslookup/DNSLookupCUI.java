package ca.ubc.cs.cs317.dnslookup;

import java.io.Console;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.net.*;
import java.util.*;

public class DNSLookupCUI implements DNSVerbosePrinter {

    public static final int MAX_INDIRECTION_LEVEL = 10;

    private static boolean verboseTracing = false;
    private static DNSLookupService lookupService;
    private static final DNSCache cache = DNSCache.getInstance();

    /**
     * Main function, called when program is first invoked.
     *
     * @param args list of arguments specified in the command line.
     */
    public static void main(String[] args) {

        InputStream instream = System.in;

        if (args.length == 1) {
            String inFileName = args[0];
            try {
                instream = new FileInputStream(inFileName);
            } catch (FileNotFoundException ignore) {

            }
        } else if (args.length > 1) {
            System.err.println("Invalid call. Usage:");
            System.err.println("\tjava -jar DNSLookupService.jar [inputFile]");
            System.err.println("where nameServer is the IP address (in dotted form) of the DNS server (potentially a root nameserver) to start the search at.");
            System.exit(1);
        }

        try {
            lookupService = new DNSLookupService(new DNSLookupCUI());
        } catch (SocketException | UnknownHostException e) {
            e.printStackTrace();
            System.exit(1);
        }

        Scanner in = new Scanner(instream);
        Console console = System.console();
        do {
            // Use console if one is available, or standard input if not.
            String commandLine;
            if (console != null) {
                System.out.print("DNSLOOKUP> ");
                commandLine = console.readLine();
            } else {
                try {
                    commandLine = in.nextLine();
                } catch (NoSuchElementException ex) {
                    break;
                }
            }

            // If reached end-of-file, leave
            if (commandLine == null) break;

            // Ignore leading/trailing spaces and anything beyond a comment character
            commandLine = commandLine.split("#", 2)[0].trim();

            // If no command shown, skip to next command
            if (commandLine.isEmpty()) continue;

            String[] commandArgs = commandLine.split(" ");

            if (commandArgs[0].equalsIgnoreCase("quit") ||
                    commandArgs[0].equalsIgnoreCase("exit"))
                break;
            else if (commandArgs[0].equalsIgnoreCase("verbose")) {
                // VERBOSE: Turn verbose setting on or off
                if (commandArgs.length == 2) {
                    if (commandArgs[1].equalsIgnoreCase("on"))
                        setVerboseTracing(true);
                    else if (commandArgs[1].equalsIgnoreCase("off"))
                        setVerboseTracing(false);
                    else {
                        System.err.println("Invalid call. Format:\n\tverbose [on|off]");
                        continue;
                    }
                } else {
                    verboseTracing = !verboseTracing;
                }
                System.out.println("Verbose tracing is now: " + (verboseTracing ? "ON" : "OFF"));
            } else if (commandArgs[0].equalsIgnoreCase("lookup") ||
                    commandArgs[0].equalsIgnoreCase("l")) {
                // LOOKUP: Find and print all results associated to a name.
                RecordType type;
                if (commandArgs.length == 2)
                    type = RecordType.A;
                else if (commandArgs.length == 3)
                    try {
                        type = RecordType.valueOf(commandArgs[2].toUpperCase());
                    } catch (IllegalArgumentException ex) {
                        System.err.println("Invalid query type. Must be one of:\n\tA, AAAA, NS, MX, CNAME");
                        continue;
                    }
                else {
                    System.err.println("Invalid call. Format:\n\tlookup hostName [type]");
                    continue;
                }
                findAndPrintResults(commandArgs[1], type);
            } else if (commandArgs[0].equalsIgnoreCase("dump")) {
                // DUMP: Print all results still cached
                cache.forEachQuestion(DNSLookupCUI::printResults);
            } else if (commandArgs[0].equalsIgnoreCase("reset")) {
                // RESET: Remove all entries from the cache
                cache.reset();
            } else {
                System.err.println("Invalid command. Valid commands are:");
                System.err.println("\tlookup fqdn [type]");
                System.err.println("\tverbose on|off");
                System.err.println("\tdump");
                System.err.println("\treset");
                System.err.println("\tquit");
            }

        } while (true);

        lookupService.close();
        System.out.println("Goodbye!");
    }

    public static void setVerboseTracing(boolean onoff) {
        verboseTracing = onoff;
    }

    /**
     * Finds all results for a host name and type and prints them on the standard output.
     *
     * @param hostName Fully qualified domain name of the host being searched.
     * @param type     Record type for search.
     */
    private static void findAndPrintResults(String hostName, RecordType type) {

        DNSQuestion question = new DNSQuestion(hostName, type, RecordClass.IN);
        try {
            Collection<CommonResourceRecord> results = lookupService.getResultsFollowingCNames(question, MAX_INDIRECTION_LEVEL);
            if (verboseTracing) System.out.println("\n========== FINAL RESULT ==========");
            printResults(question, results);
        } catch (DNSLookupService.DNSErrorException e) {
            System.out.println("Server returned an error \"" + e.getMessage() + "\" instead of a result.");
        }
    }

    /**
     * If verbose tracing is on, prints a specific query before it is sent to the server. If verbose tracing is off,
     * does nothing.
     *
     * @param question      Question parameters included in the query.
     * @param server        Nameserver expected to receive the query.
     * @param transactionID Transaction ID used for this query.
     */
    public void printQueryToSend(String protocol, DNSQuestion question, InetAddress server, int transactionID) {
        if (verboseTracing)
            System.out.printf("\n\n[Using %s] Query ID     %d %s  %s --> %s\n",
                    protocol, transactionID & 0xFFFF, question.getHostName(), question.getRecordType(),
                    server.getHostAddress());
    }

    /**
     * If verbose tracing is on, prints header information about a DNS response received from a nameserver. If verbose
     * tracing is off, does nothing.
     *
     * @param receivedTransactionId Transaction ID received from the server.
     * @param authoritative         Indicates if the response is claimed to be authoritative (true) or not (false).
     * @param error                 Error code included in the response.
     */
    public void printResponseHeaderInfo(int receivedTransactionId, boolean authoritative, boolean tc, int error) {
        if (verboseTracing)
            System.out.printf("Response ID: %d Authoritative = %b TC = %s Error = %x (%s)\n",
                    receivedTransactionId & 0xFFFF, authoritative, tc ? "true" : "false", error, DNSMessage.dnsErrorMessage(error));
    }

    /**
     * If verbose tracing is on, prints a header for the answers section of a DNS response. If verbose tracing is off,
     * does nothing. Must be called after the header info has been printed, but before the answers section is
     * processed/printed. Must be called even if there are no answers.
     *
     * @param num_answers The number of answer records included in the response.
     */
    public void printAnswersHeader(int num_answers) {
        if (verboseTracing)
            System.out.printf("  Answers (%d)\n", num_answers);
    }

    /**
     * If verbose tracing is on, prints a header for the nameservers section of a DNS response. If verbose tracing is
     * off, does nothing. Must be called after the answers section has been printed, but before the nameservers section
     * is processed/printed. Must be called even if there are no nameservers.
     *
     * @param num_nameservers The number of nameserver records included in the response.
     */
    public void printNameserversHeader(int num_nameservers) {
        if (verboseTracing)
            System.out.printf("  Nameservers (%d)\n", num_nameservers);
    }

    /**
     * If verbose tracing is on, prints a header for the additional information section of a DNS response. If verbose
     * tracing is off, does nothing. Must be called after the nameserver section has been printed, but before the
     * additional information section is processed/printed. Must be called even if there is no additional information.
     *
     * @param num_additional The number of additional information records included in the response.
     */
    public void printAdditionalInfoHeader(int num_additional) {
        if (verboseTracing)
            System.out.printf("  Additional Information (%d)\n", num_additional);
    }

    /**
     * If verbose tracing is on, prints an individual resource record received from the nameserver. If verbose tracing
     * is off, does nothing. Must be called for every record received either as an answer, a nameserver or additional
     * information, in their corresponding sections.
     *
     * @param record    The record object created for the received resource.
     * @param typeCode  Type code received by the server, to be used if the record type is not supported by the
     *                  application (if type is OTHER).
     * @param classCode Class code received by the server, to be used if the record class is not supported by the
     *                  application (if class is OTHER).
     */
    public void printIndividualResourceRecord(ResourceRecord record, int typeCode, int classCode) {
        if (verboseTracing)
            printResourceRecord(record, typeCode, classCode);
    }

    private static void printResourceRecord(ResourceRecord record, int typeCode, int classCode) {
        if (record instanceof CommonResourceRecord) {
            CommonResourceRecord rr = (CommonResourceRecord)(record);
            System.out.format("       %-30s %-10d %-5s %-5s %s\n",
                    rr.getQuestion().getHostName().isEmpty() ? "(root)" : rr.getQuestion().getHostName(),
                    rr.getRemainingTTL(),
                    rr.getRecordType() == RecordType.OTHER ? typeCode : rr.getRecordType(),
                    rr.getQuestion().getRecordClass() == RecordClass.OTHER ? classCode : rr.getQuestion().getRecordClass(),
                    rr.getTextResult());
        } else {
            assert record instanceof OPTResourceRecord;
            System.out.format("       %-30s %-10d B", "(root)", ((OPTResourceRecord) record).getPayloadSize());
        }
    }

    /**
     * Prints the result of a DNS query.
     *
     * @param question Host name and record type used for the query.
     * @param results  Collection of results to be printed for the question.
     */
    private static void printResults(DNSQuestion question, Collection<CommonResourceRecord> results) {
        if (results.isEmpty())
            printResourceRecord(new CommonResourceRecord(question, -1, "UNKNOWN"), 0, 0);
        for (CommonResourceRecord record : results)
            printResourceRecord(record, 0, 0);
    }
}
