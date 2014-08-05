package com.srch2.android.http.service;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;

abstract class Term {
    public abstract String toString();

    abstract public Term AND(Term rightTerm);

    abstract public Term OR(Term rightTerm);

    abstract public Term NOT();
}

/**
 * The SearchableTerm encapsulates the search string with all the advanced search
 * options. The default setting is copied from the
 * <code>IndexDescription</code> User can call the setting functions to override
 * the default settings.
 */
final public class SearchableTerm extends Term {

    /**
     * could be multiple words "George Lucas"
     */
    final String keywords;

    Boolean isPrefixMatching = null;
    Integer boostValue = null;
    Float fuzzySimilarity = null;
    String filterField = null;

    /**
     * Creates the term into keyword. The string passed into this function is
     * treated as a single search term. For example, if "George Lucas" is passed, the two words will be
     * treated as one word.
     * <p/>
     * The prefix, fuzziness, boostValue is set using the default value present
     * in <code>IndexDescription</code>
     *
     * @param keywords
     */
    public SearchableTerm(String keywords) {
        checkString(keywords, "search keywords is not valid");
        try {
            this.keywords = URLEncoder.encode(keywords, "UTF-8");
        } catch (UnsupportedEncodingException ignore) {
            throw new IllegalArgumentException("Unbelievable！ UTF-8 encoding is not supported! A" +
                    "re you in android platform?");
        }
    }

    static void checkString(String str, String msg) {
        if (str == null || str.length() < 0) {
            throw new IllegalArgumentException(msg);
        }
    }

    static void checkSimilarity(float similarity) {
        if (similarity < 0 || similarity > 1) {
            throw new IllegalArgumentException(
                    "similarity should be in the [0,1] range");
        }
    }

    /**
     * Overrides the prefix matching setting.
     *
     * @param isPrefixMatching disable or enable prefix match on the current query
     * @return this
     */
    public SearchableTerm setIsPrefixMatching(boolean isPrefixMatching) {
        this.isPrefixMatching = isPrefixMatching;
        return this;
    }

    /**
     * Overrides the default Index fuzziness similarity setting
     *
     * @param similarity
     * @return this
     */
    public SearchableTerm enableFuzzyMatching(float similarity) {
        checkSimilarity(similarity);
        this.fuzzySimilarity = similarity;
        return this;
    }

    /**
     * Disables the fuzzy matching
     *
     * @return this
     */
    public SearchableTerm disableFuzzyMatching() {
        this.fuzzySimilarity = -1f;
        return this;
    }

    /**
     * To boost the importance of a given term.The boost value must be a
     * positive integer, and its default value is 1.
     *
     * @param boostValue the importance of the current term, it should
     * @return
     */
    public SearchableTerm setBoostValue(int boostValue) {
        if (boostValue < 0) {
            throw new IllegalArgumentException(
                    "The boost value must be a positive integer");
        }
        this.boostValue = boostValue;
        return this;
    }

    /**
     * It specifies a field to search on. Otherwise the query is searched on all the
     * searchable fields.
     *
     * @param fieldName name of the searchable field
     * @return
     */
    public SearchableTerm searchSpecificField(String fieldName) {
        checkString(fieldName, "The fieldName is invalid");
        this.filterField = fieldName;
        return this;
    }

    /**
     * Create a composite term by two
     *
     * @param rightTerm
     * @return
     */
    public CompositeTerm AND(Term rightTerm) {
        return new CompositeTerm(BooleanOperator.AND, this, rightTerm);
    }

    public CompositeTerm OR(Term rightTerm) {
        return new CompositeTerm(BooleanOperator.OR, this, rightTerm);
    }

    public CompositeTerm NOT() {
        return new CompositeTerm(BooleanOperator.NOT, this, null);
    }

    boolean isFuzzy() {
        return fuzzySimilarity != null && fuzzySimilarity > 0
                && fuzzySimilarity < 1;
    }

    public String toString() {
        /**
         * the order of modifiers must always be prefix, boost, and then fuzzy
         */
        StringBuilder restStr = new StringBuilder();
        if (filterField != null) {
            restStr.append(filterField).append(':');
        }

        if (keywords.contains("+") || keywords.contains(" ")) {
            restStr.append('"').append(keywords).append('"');
        } else {
            restStr.append(keywords);
        }
        if (isPrefixMatching != null && isPrefixMatching) {
            restStr.append('*');
        }
        if (boostValue != null) {
            restStr.append('^').append(boostValue);
        }
        if (isFuzzy()) {
            restStr.append('~').append(fuzzySimilarity);
        }
        return restStr.toString();
    }

    enum BooleanOperator {
        AND, OR, NOT,
    }

    /**
     * The CompositeTerm that enable the boolean selection on the query terms.
     */
    final public static class CompositeTerm extends Term {

        private final BooleanOperator operator;
        private final Term left;
        private final Term right;

        CompositeTerm(BooleanOperator operator, Term left, Term right) {
            this.operator = operator;
            this.left = left;
            this.right = right;
        }

        public String toString() {
            StringBuilder sb = new StringBuilder();
            switch (operator) {
                case NOT:
                    sb.append(operator.name()).append(' ');
                    if (left.getClass() == CompositeTerm.class) {
                        sb.append('(').append(left.toString()).append(')');
                    } else {
                        sb.append(left.toString());
                    }
                    break;
                case AND:
                case OR:
                    if (left.getClass() == CompositeTerm.class) {
                        sb.append('(').append(left.toString()).append(')');
                    } else {
                        sb.append(left.toString());
                    }
                    sb.append(' ').append(operator.name()).append(' ');
                    if (left.getClass() == CompositeTerm.class) {
                        sb.append('(').append(right.toString()).append(')');
                    } else {
                        sb.append(right.toString());
                    }
                    break;
            }

            return sb.toString();
        }

        @Override
        public Term AND(Term rightTerm) {
            return new CompositeTerm(BooleanOperator.AND, this, rightTerm);
        }

        @Override
        public Term OR(Term rightTerm) {
            return new CompositeTerm(BooleanOperator.OR, this, rightTerm);
        }

        @Override
        public Term NOT() {
            return new CompositeTerm(BooleanOperator.NOT, this, null);
        }
    }
}
