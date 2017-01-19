package com.mediatek.sensorhub;
import java.util.concurrent.CopyOnWriteArrayList;

import android.os.Build;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/**
 * Condition is an useful class for configuring the condition for the {@link Action}.
 * It can be used to represent a single expression and even a complex expression.
 * Before describing the expression, some key concepts should be known.
 * <br/>
 * <table>
 * <tr>
 *     <td>Name</td>
 *     <td>Abbreviation</td>
 *     <td>Description</td>
 * </tr>
 * <tr>
 *     <td>Data Index</td>
 *     <td>"D"</td>
 *     <td>Data slot index(defined in {@link ContextInfo}).
       <br/>"D1" and "D2" represent 2 different data slot indices.</td>
 * </tr>
 * <tr>
 *     <td>Data Value</td>
 *     <td>"V"</td>
 *     <td>The value of the specified data slot index.</td>
 * </tr>
 * <tr>
 *     <td>Operator</td>
 *     <td>"op"</td>
 *     <td>Relational operator or arithmetic operator, 
 *     <br/>such as ">", ">=", "%" and so on.</td>
 * </tr>
 * <tr>
 *     <td>Combine</td>
 *     <td>"*"</td>
 *     <td>Conditional operator or bracket, such as "&&", "||" and "()".</td>
 * </tr>
 * </table>
 * 
 * <h1>Single expression:</h1>
 * <li>Single expression: D op V.
 * <br/>For example: 
 * <br/>ContextInfo.UserActivity.CURRENT_STATE = ContextInfo.UserActivity.State.ON_FOOT
 * <br/>ContextInf.Pedometer.TOTAL_DISTANCE >= 1000
 * </li>
 * 
 * <h1>Complex expressions:</h1> 
 * For the sake of simplifying the expression, "C" is used to represent a single expression. 
 * "C1" and "C2" represent 2 different single expressions.
 * <br/>The complex expression consists of several single expressions, conditional operators and(or) brackets.
 * The application can create a complex expression according to following rules:
 * <li>Combine multiple single expressions, but the maximum supported number of single expression is 4. 
 * <br/>For example, "C1 * C2 * C3 * C4" is right, but "C1 * C2 * C3 * C4 * C5" is wrong.</li>
 * <li>Multiple brackets can be used, but they should not be nested. 
 * <br/>For example, "(C1 * C2) * (C3 * C4)" is right, but "(C1 * (C2 * C3))" is wrong.</li>
 * <p/>
 * <p class="note">Note: The check process takes more time for the complex expression than the single expression. 
 * It indicates applications should create as simple expressions as they can.
 * <p/>
 */
public class Condition implements Parcelable {
    private static final String TAG = "Condition";
    private static final boolean LOG = !"user".equals(Build.TYPE) && !"userdebug".equals(Build.TYPE);
    private static final int MAX_SUPPORTED_ITEM = 4;

    /**
     * A special operator that always returns {@code true}. 
     * 
     * For example, the simple expression "D OP_ANY V" always returns {@code true}.
     */
    public static final int OP_ANY                          = 0xFF;

    /**
     * Relational operator ">".
     */
    public static final int OP_GREATER_THAN                 = 0x01;

    /**
     * Relational operator ">=".
     */
    public static final int OP_GREATER_THAN_OR_EQUALS       = 0x02;

    /**
     * Relational operator "<".
     */
    public static final int OP_LESS_THAN                    = 0x03;

    /**
     * Relational operator "<=".
     */
    public static final int OP_LESS_THAN_OR_EQUALS          = 0x04;

    /**
     * Relational operator "==".
     */
    public static final int OP_EQUALS                       = 0x05;

    /**
     * Relational operator "!=".
     */
    public static final int OP_NOT_EQUALS                   = 0x06;

    /**
     * Relational operator "%".
     */
    public static final int OP_MOD                          = 0x07;

    /*package*/ static final int COMBINE_INVALID            = 0x00;
    /*package*/ static final int COMBINE_AND                = 0x01;
    /*package*/ static final int COMBINE_OR                 = 0x02;

    private CopyOnWriteArrayList<ConditionItem> mList = new CopyOnWriteArrayList<ConditionItem>(); 

    /*package*/ Condition() {
    }

    @SuppressWarnings("unused")
    private ConditionItem[] toArray() { //called from natives
        return mList.toArray(new ConditionItem[0]);
    }

    /*package*/ String opStr(int operation) {
        switch (operation) {
        case OP_ANY:
            return "any";
        case OP_GREATER_THAN:
            return ">";
        case OP_GREATER_THAN_OR_EQUALS:
            return ">=";
        case OP_LESS_THAN:
            return "<";
        case OP_LESS_THAN_OR_EQUALS:
            return "<=";
        case OP_EQUALS:
            return "=";
        case OP_NOT_EQUALS:
            return "<>";
        case OP_MOD:
            return "%";
        default:
            return "@@";
        }
    }

    /**
     * Creates a Condition from a parcel.
     * 
     * @param source The parcel data.
     * 
     * @hide
     */
    protected Condition(Parcel source) {
        int size = source.readInt();
        mList.clear();
        for (int i = 0; i < size; i++) {
            ConditionItem item = source.readParcelable(null);
            mList.add(item);
        }
        if (LOG) {
            Log.v(TAG, "readParcel: itemSize=" + size + ", parcelSize=" + source.dataSize());
        }
    }

    /**
     * @hide
     */
    public static final Parcelable.Creator<Condition> CREATOR = new Creator<Condition>() {
        @Override
        public Condition createFromParcel(Parcel source) {
            return new Condition(source);
        }

        @Override
        public Condition[] newArray(int size) {
            return new Condition[size];
        }
    };

    /**
     * @hide
     */
    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * @hide
     */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        int itemSize = mList.size();
        dest.writeInt(itemSize);
        for (int i = 0; i < itemSize; i++) {
            dest.writeParcelable(mList.get(i), flags);
        }
        if (LOG) {
            Log.v(TAG, "writeToParcel: itemSize=" + itemSize + ",parcelSize=" + dest.dataSize());
        }
    }

    /**
     * Utility class for creating a Condition or combining Conditions.
     */
    public static class Builder {
        /**
         * Constructs a Condition.Builder instance.
         */
        public Builder() {}

        /**
         * Equivalent to {@link #createCondition(int, boolean, int, int) 
         * createCondition(index, false, operation, value)}.
         */
        public Condition createCondition(int index, int operation, int value) {
            return createCondition(index, false, operation, value);
        }

        /**
         * Equivalent to {@link #createCondition(int, boolean, int, long) 
         * createCondition(index, false, operation, value)}.
         */
        public Condition createCondition(int index, int operation, long value) {
            return createCondition(index, false, operation, value);
        }

        /**
         * Equivalent to {@link #createCondition(int, boolean, int, float) 
         * createCondition(index, false, operation, value)}.
         */
        public Condition createCondition(int index, int operation, float value) {
            return createCondition(index, false, operation, value);
        }

        /**
         * Equivalent to {@link #createCompareCondition(int, boolean, int, int, boolean) 
         * createCompareCondition(index1, false, operation, index2, false)}.
         * 
         * @hide
         */
        public Condition createCompareCondition(int index1, int operation, int index2) {
            return createCompareCondition(index1, false, operation, index2, false);
        }

        /**
         * Equivalent to {@link #createDiffCondition(int, boolean, int, int, boolean, int) 
         * createDiffCondition(index1, false, operation, index2, false, value)}.
         * 
         * @hide
         */
        public Condition createDiffCondition(int index1, int operation, int index2, int value) {
            return createDiffCondition(index1, false, operation, index2, false, value);
        }

        /**
         * Equivalent to {@link #createDiffCondition(int, boolean, int, int, boolean, long) 
         * createDiffCondition(index1, false, operation, index2, false, value)}.
         * 
         * @hide
         */
        public Condition createDiffCondition(int index1, int operation, int index2, long value) {
            return createDiffCondition(index1, false, operation, index2, false, value);
        }

        /**
         * Equivalent to {@link #createDiffCondition(int, boolean, int, int, boolean, float) 
         * createDiffCondition(index1, false, operation, index2, false, value)}.
         * 
         * @hide
         */
        public Condition createDiffCondition(int index1, int operation, int index2, float value) {
            return createDiffCondition(index1, false, operation, index2, false, value);
        }

        /**
         * Creates a single value expression instance.
         * 
         * @param index The data slot index.
         * @param isPrevious If {@code true}, checking the previous value of index. Otherwise, checking the current value.
         * @param operation The operator of the expression. Refer to {@code Condition.OP_*} for detailed information.
         * @param value The right-hand operand.
         * 
         * @return A single value expression instance.
         */
        public Condition createCondition(int index, boolean isPrevious, int operation, int value) {
            int dataType = ContextInfo.getDataType(index);
            if (DataCell.DATA_TYPE_FLOAT == dataType) {
                Log.w(TAG, "createCondition: index=" + index + " should be float instead of int!");
                return createCondition(index, isPrevious, operation, (float)value);
            } else if (DataCell.DATA_TYPE_LONG == dataType) {
                Log.w(TAG, "createCondition: index=" + index + " should be long instead of int!");
                if (ContextInfo.UserActivity.DURATION == index) {
                    return createDiffCondition(ContextInfo.Clock.TIME, isPrevious, 
                        operation, ContextInfo.UserActivity.TIMESTAMP, isPrevious, (long)value);
                } else {
                    return createCondition(index, isPrevious, operation, (long)value);
                }
            }

            ConditionItem item = new ConditionItem(index, isPrevious, operation, value);
            Condition condition = new Condition();
            condition.mList.add(item);
            return condition;
        }

        /**
         * Creates a single value expression instance.
         * 
         * @param index The data slot index.
         * @param isPrevious If {@code true}, checking the previous value of index. Otherwise, checking the current value.
         * @param operation The operator of the expression. Refer to {@code Condition.OP_*} for detailed information.
         * @param value The right-hand operand.
         * 
         * @return A single value expression instance.
         */
        public Condition createCondition(int index, boolean isPrevious, int operation, long value) {
            int dataType = ContextInfo.getDataType(index);
            if (DataCell.DATA_TYPE_FLOAT == dataType) {
                Log.w(TAG, "createCondition: index=" + index + " should be float instead of long!");
                return createCondition(index, isPrevious, operation, (float)value);
            } else if (DataCell.DATA_TYPE_INT == dataType) {
                Log.w(TAG, "createCondition: index=" + index + " should be int instead of long!");
                return createCondition(index, isPrevious, operation, (int)value);
            }
            if (ContextInfo.UserActivity.DURATION == index) {
                return createDiffCondition(ContextInfo.Clock.TIME, isPrevious, 
                    operation, ContextInfo.UserActivity.TIMESTAMP, isPrevious, value);
            }
            ConditionItem item = new ConditionItem(index, isPrevious, operation, value);
            Condition condition = new Condition();
            condition.mList.add(item);
            return condition;
        }

        /**
         * Creates a single value expression instance.
         * 
         * @param index The data slot index.
         * @param isPrevious If {@code true}, checking the previous value of index. Otherwise, checking the current value.
         * @param operation The operator of the expression. Refer to {@code Condition.OP_*} for detailed information.
         * @param value The right-hand operand.
         * 
         * @return A single value expression instance.
         */
        public Condition createCondition(int index, boolean isPrevious, int operation, float value) {
            int dataType = ContextInfo.getDataType(index);
            if (DataCell.DATA_TYPE_INT == dataType) {
                Log.w(TAG, "createCondition: index=" + index + " should be int instead of float!");
                return createCondition(index, isPrevious, operation, (int)value);
            } else if (DataCell.DATA_TYPE_LONG == dataType) {
                Log.w(TAG, "createCondition: index=" + index + " should be long instead of float!");
                if (ContextInfo.UserActivity.DURATION == index) {
                    return createDiffCondition(ContextInfo.Clock.TIME, isPrevious, 
                        operation, ContextInfo.UserActivity.TIMESTAMP, isPrevious, (long)value);
                } else {
                    return createCondition(index, isPrevious, operation, (long)value);
                }
            }
            ConditionItem item = new ConditionItem(index, isPrevious, operation, value);
            Condition condition = new Condition();
            condition.mList.add(item);
            return condition;
        }

        /**
         * Creates a single comparison expression instance.
         * 
         * @param index1 The slot index of the first data.
         * @param isPrevious1 If {@code true}, checking the previous value of index1. Otherwise, checking the current value.
         * @param operation The operator of the expression. Refer to {@code Condition.OP_*} for detailed information.
         * @param index2 The slot index of the second data.
         * @param isPrevious2 If {@code true}, checking the previous value of index2. Otherwise, checking the current value.
         * 
         * @return A single comparison instance.
         * 
         * @hide
         */
        public Condition createCompareCondition(int index1, boolean isPrevious1, int operation, 
                int index2, boolean isPrevious2) {
            ConditionItem item = new ConditionItem(index1, isPrevious1, operation, index2, isPrevious2);
            Condition condition = new Condition();
            condition.mList.add(item);
            return condition;
        }

        /**
         * Creates a single difference expression instance.
         * 
         * @param index1 The slot index of the first data.
         * @param isPrevious1 If {@code true}, checking the previous value of index1. Otherwise, checking the current value.
         * @param operation The operator of the expression. Refer to {@code Condition.OP_*} for detailed information.
         * @param index2 The slot index of the second data.
         * @param isPrevious2 If {@code true}, checking the previous value of index2. Otherwise, checking the current value.
         * @param value The right-hand operand. 
         * 
         * @return A single difference instance.
         * 
         * @hide
         */
        public Condition createDiffCondition(int index1, boolean isPrevious1, int operation, 
                int index2, boolean isPrevious2, int value) {
            ConditionItem item = new ConditionItem(index1, isPrevious1, operation, index2, isPrevious2, value);
            Condition condition = new Condition();
            condition.mList.add(item);
            return condition;
        }

        /**
         * Creates a single difference expression instance.
         * 
         * @param index1 The slot index of the first data.
         * @param isPrevious1 If {@code true}, checking the previous value of index1. Otherwise, checking the current value.
         * @param operation The operator of the expression. Refer to {@code Condition.OP_*} for detailed information.
         * @param index2 The slot index of the second data.
         * @param isPrevious2 If {@code true}, checking the previous value of index2. Otherwise, checking the current value.
         * @param value The right-hand operand. 
         * 
         * @return A single difference instance.
         * 
         * @hide
         */
        public Condition createDiffCondition(int index1, boolean isPrevious1, int operation, 
                int index2, boolean isPrevious2, long value) {
            ConditionItem item = new ConditionItem(index1, isPrevious1, operation, index2, isPrevious2, value);
            Condition condition = new Condition();
            condition.mList.add(item);
            return condition;
        }

        /**
         * Creates a single difference expression instance.
         * 
         * @param index1 The slot index of the first data.
         * @param isPrevious1 If {@code true}, checking the previous value of index1. Otherwise, checking the current value.
         * @param operation The operator of the expression. Refer to {@code Condition.OP_*} for detailed information.
         * @param index2 The slot index of the second data.
         * @param isPrevious2 If {@code true}, checking the previous value of index2. Otherwise, checking the current value.
         * @param value The right-hand operand. 
         * 
         * @return A single difference instance.
         * 
         * @hide
         */
        public Condition createDiffCondition(int index1, boolean isPrevious1, int operation, 
                int index2, boolean isPrevious2, float value) {
            ConditionItem item = new ConditionItem(index1, isPrevious1, operation, index2, isPrevious2, value);
            Condition condition = new Condition();
            condition.mList.add(item);
            return condition;
        }

        /**
         * Creates a complex expression instance by combining two instances with "&&".
         * 
         * @param c1 The first instance.
         * @param c2 The second instance.
         * 
         * @return A new complex instance "c1 && c2".
         */
        public Condition combineWithAnd(Condition c1, Condition c2) {
            if (null == c1 || null == c2) {
                if(LOG) Log.e(TAG, "combineWithAnd: null condition! c1=" + c1 + ", c2=" + c2);
                return null;
            }
            int size1 = c1.mList.size();
            int size2 = c2.mList.size();
            if (size1 == 0) {
                return c2;
            } else if (size2 == 0) {
                return c1;
            } else if (size1 + size2 > MAX_SUPPORTED_ITEM) {
                if (LOG) Log.e(TAG, "combineWithAnd: itemSize > 4! size1=" + size1 + ",size2=" + size2);
                return null;
            }
            Condition condition = new Condition();
            condition.mList.addAll(c1.mList);
            condition.mList.addAll(c2.mList);
            condition.mList.get(size1).setCombine(COMBINE_AND);
            return condition;
        }

        /**
         * Creates a complex expression instance by combining two instances with "||".
         * 
         * @param c1 The first instance.
         * @param c2 The second instance.
         * 
         * @return A new complex instance "c1 || c2".
         */
        public Condition combineWithOr(Condition c1, Condition c2) {
            if (null == c1 || null == c2) {
                if (LOG) Log.e(TAG, "combineWithOr: null condition! c1=" + c1 + ", c2=" + c2);
                return null;
            }
            int size1 = c1.mList.size();
            int size2 = c2.mList.size();
            if (size1 == 0) {
                return c2;
            } else if (size2 == 0) {
                return c1;
            } else if (size1 + size2 > MAX_SUPPORTED_ITEM) {
                if (LOG) Log.e(TAG, "combineWithOr: itemSize > 4! size1=" + size1 + ",size2=" + size2);
                return null;
            }
            Condition condition = new Condition();
            condition.mList.addAll(c1.mList);
            condition.mList.addAll(c2.mList);
            condition.mList.get(size1).setCombine(COMBINE_OR);
            return condition;
        }

        /**
         * Creates a complex expression instance by adding brackets to a single expression.
         * 
         * @param c The single expression instance to be bracketed.
         * 
         * @return A new complex expression instance with c bracketed, namely "(c)".
         */
        public Condition combineWithBracket(Condition c) {
            if (null == c) {
                if (LOG) Log.e(TAG, "combineWithBracket: null conditon!");
                return null;
            } else {
                int size = c.mList.size();
                if (size == 0) {
                    if (LOG) Log.w(TAG, "combineWithBracket: empty condition!");
                    return c;
                }
                for (int i = 0; i < size; i++) {
                    if (c.mList.get(i).isBracketLeft()) {
                        if(LOG) Log.e(TAG, "combineWithBracket: do not support nest bracket. item[" 
                                    + i + "]=" + c.mList.get(i));
                        return null;
                    }
                }
            }
            Condition condition = new Condition();
            condition.mList.addAll(c.mList);
            condition.mList.get(0).setBracketLeft();
            condition.mList.get(condition.mList.size() - 1).setBracketRight();
            return condition;
        }
    }
}
