package ca.ubc.cs317.dict.ui;

import ca.ubc.cs317.dict.model.Definition;

import javax.swing.table.AbstractTableModel;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * Created by Jonatan on 2017-09-09.
 */
public class DefinitionTableModel extends AbstractTableModel {

    private List<Definition> definitionList = new ArrayList<>();

    /**
     * Returns the number of rows in the model. A
     * <code>JTable</code> uses this method to determine how many rows it
     * should display.  This method should be quick, as it
     * is called frequently during rendering.
     *
     * @return the number of rows in the model
     * @see #getColumnCount
     */
    @Override
    public int getRowCount() {
        return definitionList.size();
    }

    /**
     * Returns the number of columns in the model. A
     * <code>JTable</code> uses this method to determine how many columns it
     * should create and display by default.
     *
     * @return the number of columns in the model
     * @see #getRowCount
     */
    @Override
    public int getColumnCount() {
        return 3;
    }

    /**
     * Returns the value for the cell at <code>columnIndex</code> and
     * <code>rowIndex</code>.
     *
     * @param rowIndex    the row whose value is to be queried
     * @param columnIndex the column whose value is to be queried
     * @return the value Object at the specified cell
     */
    @Override
    public Object getValueAt(int rowIndex, int columnIndex) {
        Definition definition = definitionList.get(rowIndex);
        switch (columnIndex) {
            case 0:
                return definition.getWord();
            case 1:
                return definition.getDatabaseName();
            case 2:
                return definition.getDefinition();
        }
        return null;
    }

    @Override
    public String getColumnName(int column) {
        switch (column) {
            case 0: return "Word";
            case 1: return "Database";
            case 2: return "Definition";
            default: return null;
        }
    }

    public void populateDefinitions(Collection<Definition> definitions) {
        definitionList.clear();
        definitionList.addAll(definitions);
        fireTableDataChanged();
    }
}
