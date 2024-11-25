package ca.ubc.cs317.dict.model;

/**
 * Created by Jonatan on 2017-09-09.
 */
public class MatchingStrategy {
    private String name;
    private String description;

    public MatchingStrategy(String name, String description) {
        this.name = name;
        this.description = description;
    }

    public String getName() {
        return name;
    }

    public String getDescription() {
        return description;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        MatchingStrategy that = (MatchingStrategy) o;

        return name != null ? name.equals(that.name) : that.name == null;
    }

    @Override
    public int hashCode() {
        return name != null ? name.hashCode() : 0;
    }

    @Override
    public String toString() {
        return description;
    }
}
