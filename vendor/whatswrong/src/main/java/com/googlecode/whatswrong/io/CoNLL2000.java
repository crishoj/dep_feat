package com.googlecode.whatswrong.io;

import com.googlecode.whatswrong.NLPInstance;

import java.util.List;

/**
 * Loads CoNLL 2000 chunk data.
 *
 * @author Sebastian Riedel
 */
public class CoNLL2000 implements TabProcessor {

    /**
     * The name of the processor.
     */
    public static final String name = "CoNLL 2000";


    /**
     * Returns the name of this processor.
     *
     * @return the name of this processor.
     */
    public String toString() {
        return name;
    }

    /**
     * @see TabProcessor#create(List<? extends List<String>>)
     */
    public NLPInstance create(List<? extends List<String>> rows) {

        NLPInstance instance = new NLPInstance();
        int index = 0;
        for (List<String> row : rows) {
            String chunk = row.get(2);
            instance.addToken().
                addProperty("Word", row.get(0)).
                addProperty("Index", String.valueOf(index));

            instance.addSpan(index, index, row.get(1), "pos");
            instance.addSpan(index, index, chunk, "chunk (BIO)");
            ++index;
        }

        TabFormat.extractSpan00(rows, 2, "chunk", instance);

        return instance;
    }


    /**
     * @see TabProcessor#createOpen(List<? extends List<String>>)
     */
    public NLPInstance createOpen(List<? extends List<String>> rows) {
        return null;
    }

    /**
     * @see TabProcessor#supportsOpen()
     */
    public boolean supportsOpen() {
        return false;
    }
}