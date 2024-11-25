package ca.ubc.cs317.dict.ui;

import ca.ubc.cs317.dict.net.DictConnectionException;
import ca.ubc.cs317.dict.model.Database;
import ca.ubc.cs317.dict.model.MatchingStrategy;
import ca.ubc.cs317.dict.net.DictionaryConnection;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Collection;
import java.util.Collections;
import java.util.Objects;
import java.util.concurrent.ExecutionException;

/**
 * Created by Jonatan on 2017-09-09.
 */
public class DictionaryMain extends JFrame {

    private DictionaryConnection connection;
    private String serverName = "dict.org";

    private final DefaultComboBoxModel<Database> databaseModel;
    private final DefaultComboBoxModel<MatchingStrategy> strategyModel;
    private final DefinitionTableModel definitionModel;

    private final WordSearchField wordSearchField;
    private final JTable definitionTable;

    private final JEditorPane databaseDescription;

    DictionaryMain() {
        super("Dictionary");
        this.setSize(800, 600);
        this.addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                if (connection != null)
                    connection.close();
            }
        });
        this.setDefaultCloseOperation(EXIT_ON_CLOSE);

        JPanel optionsPanel = new JPanel(new GridBagLayout());
        GridBagConstraints c = new GridBagConstraints();
        c.fill = GridBagConstraints.BOTH;
        c.weightx = 1;
        this.getContentPane().add(optionsPanel, BorderLayout.SOUTH);

        databaseModel = new DefaultComboBoxModel<>();
        strategyModel = new DefaultComboBoxModel<>();

        JLabel databaseLabel = new JLabel("Database:");
        databaseLabel.setHorizontalAlignment(JLabel.TRAILING);
        JComboBox<Database> databaseSelection = new JComboBox<>(databaseModel);
        databaseLabel.setLabelFor(databaseSelection);
        c.gridwidth = GridBagConstraints.RELATIVE;
        optionsPanel.add(databaseLabel, c);
        c.gridwidth = GridBagConstraints.REMAINDER;
        optionsPanel.add(databaseSelection, c);

        databaseSelection.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Database d = (Database) databaseModel.getSelectedItem();
                if (d != null && connection != null) {
                    try {
                        databaseDescription.setText(d.getDescription() + "\n" + connection.getDatabaseInfo(d));
                    } catch (DictConnectionException ex) {
                        databaseDescription.setText(d.getDescription() + "\n" + "Error retrieving database information");
                    }
                }
            }
        });
        JLabel strategyLabel = new JLabel("Hint Strategy:");
        strategyLabel.setHorizontalAlignment(JLabel.TRAILING);
        JComboBox<MatchingStrategy> strategySelection = new JComboBox<>(strategyModel);
        strategyLabel.setLabelFor(strategySelection);
        c.gridwidth = GridBagConstraints.RELATIVE;
        optionsPanel.add(strategyLabel, c);
        c.gridwidth = GridBagConstraints.REMAINDER;
        optionsPanel.add(strategySelection, c);

        databaseDescription = new JEditorPane();
        databaseDescription.setContentType("text");
        databaseDescription.setText("All databases");
        databaseDescription.setPreferredSize(new Dimension(400, 100));
        optionsPanel.add(new JScrollPane(databaseDescription), c);

        JButton disconnectButton = new JButton("Disconnect");
        disconnectButton.addActionListener(e -> establishConnection());
        c.gridwidth = GridBagConstraints.REMAINDER;
        optionsPanel.add(disconnectButton, c);

        JPanel searchPanel = new JPanel(new BorderLayout());
        this.getContentPane().add(searchPanel, BorderLayout.NORTH);

        wordSearchField = new WordSearchField(this);
        searchPanel.add(wordSearchField, BorderLayout.CENTER);

        JButton searchButton = new JButton("Search");
        searchButton.addActionListener(e -> showDefinitions());
        this.getRootPane().setDefaultButton(searchButton);
        searchPanel.add(searchButton, BorderLayout.LINE_END);

        definitionModel = new DefinitionTableModel();
        definitionTable = new JTable(definitionModel);
        definitionTable.getColumnModel().getColumn(2).setCellRenderer((table, value, isSelected, hasFocus, row, column) -> {
            JTextArea area = new JTextArea(value.toString());
            area.setBackground(UIManager.getColor(isSelected ? "Table.selectionBackground" : "Table.background"));
            return area;
        });
        definitionTable.getColumnModel().getColumn(0).setPreferredWidth(30);
        definitionTable.getColumnModel().getColumn(1).setPreferredWidth(30);
        definitionTable.getColumnModel().getColumn(2).setPreferredWidth(500);
        this.getContentPane().add(new JScrollPane(definitionTable), BorderLayout.CENTER);
    }

    public void handleException(Throwable ex) {
        JOptionPane.showMessageDialog(this, "Connection error:\n" + ex.toString(), "Connection error", JOptionPane.ERROR_MESSAGE);
        establishConnection();
    }

    public void showDefinitions() {

        new SwingWorker<Void, Void>() {
            private final String word = Objects.requireNonNullElse(wordSearchField.getSelectedItem(), "").toString();

            @Override
            protected Void doInBackground() throws Exception {
                definitionModel.populateDefinitions(connection.getDefinitions(word,
                        (Database) databaseModel.getSelectedItem()));
                return null;
            }

            @Override
            protected void done() {
                try {
                    get(); // Just to trigger a possible exception caused by doInBackground
                    for (int i = 0; i<definitionModel.getRowCount() ; i++) {
                        Component c = definitionTable.prepareRenderer(definitionTable.getCellRenderer(i, 2), i, 2);
                        definitionTable.setRowHeight(i, Math.max((int) c.getPreferredSize().getHeight(), definitionTable.getRowHeight()));
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                } catch (ExecutionException e) {
                    handleException(e.getCause());
                }
            }
        }.execute();

    }

    public void establishConnection() {
        if (connection != null)
            connection.close();

        definitionModel.populateDefinitions(Collections.emptyList());
        databaseModel.removeAllElements();
        databaseModel.addElement(new Database("*", "All databases"));
        databaseModel.addElement(new Database("!", "Any database"));
        strategyModel.removeAllElements();
        wordSearchField.reset();

        try {
            serverName = JOptionPane.showInputDialog(this, "Dictionary server",
                    serverName);
            if (serverName == null) System.exit(0);

            if (serverName.contains(":")) {
                String[] serverData = serverName.split(":", 2);
                connection = new DictionaryConnection(serverData[0], Integer.parseInt(serverData[1]));
            } else
                connection = new DictionaryConnection(serverName);

            for (Database db : connection.getDatabaseList().values()) {
                databaseModel.addElement(db);
            }

            for (MatchingStrategy strategy : connection.getStrategyList()) {
                strategyModel.addElement(strategy);
                if (strategy.getName().equals("prefix"))
                    strategyModel.setSelectedItem(strategy);
            }
        } catch (DictConnectionException ex) {
            handleException(ex);
        }

        wordSearchField.grabFocus();
    }

    public Collection<String> getMatchList(String word) throws DictConnectionException {
        return connection.getMatchList(word,
                (MatchingStrategy) strategyModel.getSelectedItem(),
                (Database) databaseModel.getSelectedItem());
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            DictionaryMain main = new DictionaryMain();
            main.setVisible(true);
            main.establishConnection();
        });
    }

}
