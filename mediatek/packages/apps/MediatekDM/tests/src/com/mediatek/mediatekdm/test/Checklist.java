
package com.mediatek.mediatekdm.test;

import android.util.Log;
import junit.framework.TestCase;
import java.util.HashMap;
import java.util.Map;

public class Checklist {
    private static class Pair<T extends Object> {
        public final T expected;
        public T actual;

        public Pair(T e) {
            expected = e;
        }
    }

    public static final Object NilObject = new Object();
    private Map<String, Pair<Object>> mMapping = new HashMap<String, Checklist.Pair<Object>>();

    public void addCheckItem(String key, Object expected) {
        mMapping.put(key, new Pair<Object>(expected == null ? NilObject : expected));
    }

    public void fillCheckItem(String key, Object actual) {
        if (!mMapping.containsKey(key)) {
            Pair<Object> pair = new Pair<Object>(NilObject);
            pair.actual = (actual == null) ? NilObject : actual;
            mMapping.put(key, pair);
        } else {
            Pair<Object> pair = mMapping.get(key);
            pair.actual = (actual == null) ? NilObject : actual;
        }
    }

    public void check() {
        for (String key : mMapping.keySet()) {
            Log.d("MDMTest", "Check Item: " + key);
            Pair<Object> pair = mMapping.get(key);
            TestCase.assertNotSame(NilObject, pair.expected);
            TestCase.assertNotSame(NilObject, pair.actual);
            TestCase.assertEquals(pair.expected, pair.actual);
        }
    }
}
