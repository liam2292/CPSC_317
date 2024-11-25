package ca.ubc.cs317.dict.ui;

import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.plaf.metal.MetalComboBoxEditor;
import java.util.LinkedHashSet;
import java.util.Set;
import java.util.concurrent.ExecutionException;

/**
 * Created by Jonatan on 2017-09-10.
 */
public class WordSearchField extends JComboBox<String> implements DocumentListener {

    private DictionaryMain main;
    private JTextField textField;

    private DefaultComboBoxModel<String> model;

    public WordSearchField(DictionaryMain main) {

        this.setModel(model = new DefaultComboBoxModel<>());
        this.main = main;

        setEditable(true);
        setEditor(new MetalComboBoxEditor() {

            @Override
            public void setItem(Object newItem) {
                if (newItem != null &&
                        !newItem.equals(((JTextField) getEditorComponent()).getText())) {
                    super.setItem(newItem);
                    // WordSearchField.this.main.showDefinitions();
                }
            }
        });
        textField = (JTextField) getEditor().getEditorComponent();
        textField.getDocument().addDocumentListener(this);
    }

    public void reset() {
        model.removeAllElements();
        textField.setText("");
    }

    @Override
    public void insertUpdate(DocumentEvent e) {
        showSuggestions();
    }

    @Override
    public void removeUpdate(DocumentEvent e) {
        showSuggestions();
    }

    @Override
    public void changedUpdate(DocumentEvent e) {
        showSuggestions();
    }


    public void showSuggestions() {
        final String typed = textField.getText();
        model.removeAllElements();
        if (typed.isEmpty())
            return;

        new SwingWorker<Set<String>, String>() {
            String word = typed;

            @Override
            protected Set<String> doInBackground() throws Exception {
                Set<String> matches = new LinkedHashSet<>();
                matches.add(word);
                matches.addAll(main.getMatchList(word));
                return matches;
            }

            @Override
            protected void done() {
                // If user typed another character since this worker started, stop
                if (!textField.getText().equals(word)) return;
                try {
                    for (String match : this.get()) {
                        model.addElement(match);
                    }
                    if (model.getSize() > 1)
                        showPopup();
                    else
                        hidePopup();
                } catch (ExecutionException e) {
                    main.handleException(e.getCause());
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }.execute();
    }
}
