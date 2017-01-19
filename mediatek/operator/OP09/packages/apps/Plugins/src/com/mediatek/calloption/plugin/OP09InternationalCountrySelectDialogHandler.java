package com.mediatek.calloption.plugin;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.telephony.PhoneNumberUtils;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ExpandableListView;
import android.widget.RadioButton;
import android.widget.SearchView;
import android.widget.SimpleExpandableListAdapter;
import android.widget.TextView;

import com.android.i18n.phonenumbers.PhoneNumberUtil;
import com.mediatek.op09.plugin.R;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

public class OP09InternationalCountrySelectDialogHandler implements /*View.OnClickListener, */
                                                                    SearchView.OnQueryTextListener,
                                                                    ExpandableListView.OnChildClickListener {

    private static final String TAG = "OP09InternationalCountrySelectDialogHandler";

    private static final String MAP_KEY_NAME_COUNTRY_ISO      = "CountryISO";
    private static final String MAP_KEY_NAME_COUNTRY_CODE     = "CountryCode";
    private static final String MAP_KEY_NAME_COUNTRY_NAME     = "CountryName";
    private static final String MAP_KEY_NAME_CONTINENT_CODE   = "ContinentCode";
    private static final String MAP_KEY_NAME_CONTINENT_NAME   = "ContinentName";
    private static final String MAP_KEY_NAME_CONTINENT_EXPAND = "Expand";

    private static final String CONTINENT_CODE_ASIA = "142";
    private static final String CONTINENT_CODE_WORLD = "001";

    public interface OnCountrySelectListener {
        void onCountrySelected(String countryISO, String countryCode, String countryName);
    }

    private Context mActivityContext;
    private Context mPluginContext;
    private OnCountrySelectListener mCountrySelectListener;
    private SimpleExpandableListAdapter mListAdapter;

    private List<Map<String, String>> mContinentInfoMapList;
    private List<List<Map<String, String>>> mCountryInfoMapList;

    private SearchView mSearchView;
    private View mDialogView;
    private Dialog mDialog;
    private ExpandableListView mCountryListView;
    private TextView mNoSearchResult;

    private boolean mIsInflated;

    public OP09InternationalCountrySelectDialogHandler(Context activityContext, Context pluginContext,
                                                       OnCountrySelectListener countrySelectListener) {
        mActivityContext = activityContext;
        mPluginContext = pluginContext;
        mCountrySelectListener = countrySelectListener;
        mContinentInfoMapList = new ArrayList<Map<String, String>>();
        mCountryInfoMapList = new ArrayList<List<Map<String, String>>>();
    }

    public void showCountrySelectDialog() {
        mContinentInfoMapList.removeAll(mContinentInfoMapList);
        mCountryInfoMapList.removeAll(mCountryInfoMapList);
        addSupportedCountryInfoToMapList(mContinentInfoMapList, mCountryInfoMapList);
        if (mIsInflated) {
            mSearchView.setQuery("", false);
        } else {
            initDialogView();
            mIsInflated = true;
            AlertDialog.Builder builder = new AlertDialog.Builder(mActivityContext);
            builder.setView(mDialogView);
            mDialog = builder.create();
        }
        mSearchView.clearFocus();
        mNoSearchResult.setVisibility(View.GONE);
        mCountryListView.setVisibility(View.VISIBLE);
        expandSpecifiedContinentOfCountrySelectList(CONTINENT_CODE_ASIA);
        mDialog.show();
    }

    private void initDialogView() {
        mDialogView = LayoutInflater.from(mPluginContext).inflate(R.layout.international_country_select_list, null, false);

        mCountryListView = (ExpandableListView) mDialogView.findViewById(R.id.list);

        mListAdapter = new SimpleExpandableListAdapter(
                            mPluginContext, mContinentInfoMapList,
                            R.layout.international_country_select_continent_list_item,
                            new String[] { MAP_KEY_NAME_CONTINENT_NAME },
                            new int[] { R.id.continent_name },
                            mCountryInfoMapList,
                            R.layout.international_country_select_country_list_item,
                            new String[] { MAP_KEY_NAME_COUNTRY_NAME, MAP_KEY_NAME_COUNTRY_CODE },
                            new int[] { R.id.country_name, R.id.country_code });

        mCountryListView.setAdapter(mListAdapter);
        mCountryListView.setOnChildClickListener(this);

        mSearchView = (SearchView) mDialogView.findViewById(R.id.search);
        mSearchView.setImeOptions(EditorInfo.IME_ACTION_SEARCH);
        mSearchView.setIconifiedByDefault(true);
        mSearchView.setQueryHint(mPluginContext.getString(com.android.internal.R.string.search_go));
        mSearchView.setIconified(false);
        mSearchView.setOnQueryTextListener(this);

        mNoSearchResult = (TextView) mDialogView.findViewById(R.id.no_search_result);
    }

    @Override
    public boolean onQueryTextSubmit(String query) {
        return false;
    }

    @Override
    public boolean onQueryTextChange(String inputText) {
        mContinentInfoMapList.removeAll(mContinentInfoMapList);
        mCountryInfoMapList.removeAll(mCountryInfoMapList);
        if (!TextUtils.isEmpty(inputText)) {
            List<Map<String, String>> continentInfoMapListForSearch = new ArrayList<Map<String, String>>();
            List<List<Map<String, String>>> countryInfoMapListForSearch = new ArrayList<List<Map<String, String>>>();
            addSupportedCountryInfoToMapList(continentInfoMapListForSearch, countryInfoMapListForSearch);
            searchCountryInfoFromMap(inputText, continentInfoMapListForSearch, countryInfoMapListForSearch,
                                     mContinentInfoMapList, mCountryInfoMapList);
            expandAllContinentsOfCountrySelectList();
        } else {
            addSupportedCountryInfoToMapList(mContinentInfoMapList, mCountryInfoMapList);
            expandSpecifiedContinentOfCountrySelectList(CONTINENT_CODE_ASIA);
        }
        if (0 == mCountryInfoMapList.size()) {
            mNoSearchResult.setVisibility(View.VISIBLE);
            mCountryListView.setVisibility(View.GONE);
        } else {
            mNoSearchResult.setVisibility(View.GONE);
            mCountryListView.setVisibility(View.VISIBLE);
        }
        mListAdapter.notifyDataSetChanged();
        return false;
    }

    public boolean onChildClick(ExpandableListView parent, View v, int groupPosition,
                                int childPosition, long id) {
        Map<String, Object> item = (Map<String, Object>) parent.getExpandableListAdapter().getChild(groupPosition, childPosition);
        mCountrySelectListener.onCountrySelected((String)item.get(MAP_KEY_NAME_COUNTRY_ISO),
                // remove "+"
                ((String)item.get(MAP_KEY_NAME_COUNTRY_CODE)).substring(1),
                (String)item.get(MAP_KEY_NAME_COUNTRY_NAME));
        if (null != mDialog) {
            mDialog.dismiss();
        }
        return false;
    }

    public void onHandledDialogDismiss() {
        if (null != mDialog) {
            mDialog.dismiss();
        }
    }

    private void searchCountryInfoFromMap(String searchText, List<Map<String, String>> continentInfoMapListForSearch,
                                          List<List<Map<String, String>>> countryInfoMapListForSearch,
                                          List<Map<String, String>> continentInfoMapListResult,
                                          List<List<Map<String, String>>> countryInfoMapListResult) {
        Iterator iteratorCountryInfoList = countryInfoMapListForSearch.iterator();
        Iterator iteratorContinentInfo = continentInfoMapListForSearch.iterator();
        for (;iteratorCountryInfoList.hasNext() && iteratorContinentInfo.hasNext();) {
            List<Map<String, String>> subCountryInfoMapListForSearch
                    = (List<Map<String, String>>) iteratorCountryInfoList.next();
            Map<String, String> continentInfoMapItemForSearch = (Map<String, String>) iteratorContinentInfo.next();
            List<Map<String, String>> subCountryInfoMapListResult = new ArrayList<Map<String, String>>();
            for (Iterator iteratorCountryInfoSubList = subCountryInfoMapListForSearch.iterator();
                                                               iteratorCountryInfoSubList.hasNext();) {
                Map<String, String> iteratorCountryInfoItem = (Map<String, String>) iteratorCountryInfoSubList.next();
                String countryName = iteratorCountryInfoItem.get(MAP_KEY_NAME_COUNTRY_NAME);
                String countryCode = iteratorCountryInfoItem.get(MAP_KEY_NAME_COUNTRY_CODE);
                if (countryName.toUpperCase().contains(searchText.toUpperCase()) || countryCode.contains(searchText)) {
                    if (!isSameMapItemExits(continentInfoMapListResult, MAP_KEY_NAME_CONTINENT_NAME,
                                            continentInfoMapItemForSearch.get(MAP_KEY_NAME_CONTINENT_NAME))) {
                        Map<String, String> continentInfoItemResult = new HashMap<String, String>();
                        continentInfoItemResult.put(MAP_KEY_NAME_CONTINENT_NAME,
                                                    continentInfoMapItemForSearch.get(MAP_KEY_NAME_CONTINENT_NAME));
                        continentInfoMapListResult.add(continentInfoItemResult);
                    }
                    Map<String, String> countryInfoMapItemResult = new HashMap<String, String>();
                    countryInfoMapItemResult.put(MAP_KEY_NAME_COUNTRY_ISO,
                                                 iteratorCountryInfoItem.get(MAP_KEY_NAME_COUNTRY_ISO));
                    countryInfoMapItemResult.put(MAP_KEY_NAME_COUNTRY_CODE, countryCode);
                    countryInfoMapItemResult.put(MAP_KEY_NAME_COUNTRY_NAME, countryName);
                    subCountryInfoMapListResult.add(countryInfoMapItemResult);
                }
            }
            if (subCountryInfoMapListResult.size() > 0) {
                countryInfoMapListResult.add(subCountryInfoMapListResult);
            }
        }
    }

    private void expandAllContinentsOfCountrySelectList() {
        int count = mListAdapter.getGroupCount();
        for (int i = 0; i < count; ++ i) {
            mCountryListView.expandGroup(i);
        }
    }

    private void expandSpecifiedContinentOfCountrySelectList(String continentCodeToExpand) {
        int count = mListAdapter.getGroupCount();
        for (int i = 0; i < count; ++ i) {
            Map<String, String> continentItem = (Map<String, String>) mListAdapter.getGroup(i);
            if (null != continentItem) {
                if (continentCodeToExpand.equals(continentItem.get(MAP_KEY_NAME_CONTINENT_CODE))) {
                    mCountryListView.expandGroup(i);
                } else {
                    mCountryListView.collapseGroup(i);
                }
            }
        }
    }

    private void addSupportedCountryInfoToMapList(List<Map<String, String>> continentInfoMapList,
                                                  List<List<Map<String, String>>> countryInfoMapList) {
        Map<Integer, List<String>> regionMap = PhoneNumberUtil.getInstance().getCountryCodeToRegionCodeMap();
        Set<Integer> countryCodeKeySet = regionMap.keySet();

        for (Iterator iterator = countryCodeKeySet.iterator(); iterator.hasNext();) {
            Integer countryCode = (Integer) iterator.next();
            List<String> regionCodeList = regionMap.get(countryCode);
            for (Iterator iterator2 = regionCodeList.iterator(); iterator2.hasNext();) {
                String countryISO = (String)iterator2.next();
                Locale localeCountry = new Locale(Locale.getDefault().getLanguage(), countryISO);
                if (CONTINENT_CODE_WORLD.equals("localeCountry.getContinent()")) {
                    continue;
                }
                Map<String, String> itemCountry = new HashMap<String, String>();
                itemCountry.put(MAP_KEY_NAME_COUNTRY_ISO, countryISO);
                itemCountry.put(MAP_KEY_NAME_COUNTRY_CODE, "+" + String.valueOf(countryCode));
                itemCountry.put(MAP_KEY_NAME_COUNTRY_NAME, localeCountry.getDisplayCountry(Locale.getDefault()));
                List<Map<String, String>> itemCountryInfoMapList =
                        findSameContinentCountryListItem(continentInfoMapList, countryInfoMapList,
                                                         "localeCountry.getContinent()");
                if (null == itemCountryInfoMapList) {
                    // add new continent
                    Map<String, String> itemContinent = new HashMap<String, String>();
                    Locale localeContinent = new Locale(Locale.getDefault().getLanguage(), "localeCountry.getContinent()");
                    itemContinent.put(MAP_KEY_NAME_CONTINENT_NAME, localeContinent.getDisplayCountry(Locale.getDefault()));
                    itemContinent.put(MAP_KEY_NAME_CONTINENT_CODE, "localeCountry.getContinent()");
                    continentInfoMapList.add(itemContinent);
                    // add country
                    itemCountryInfoMapList = new ArrayList<Map<String, String>>();
                    countryInfoMapList.add(itemCountryInfoMapList);
                }
                itemCountryInfoMapList.add(itemCountry);
            }
        }
    }

    private boolean isSameMapItemExits(List<Map<String, String>> countryInfoMapList, String itemKey, String itemValue) {
        ListIterator<Map<String, String>> iterator = countryInfoMapList.listIterator();
        while (iterator.hasNext()) {
            Map<String, String> itemMap = (Map<String, String>)iterator.next();
            if (itemMap.get(itemKey).equals(itemValue)) {
                return true;
            }
        }
        return false;
    }

    private List<Map<String, String>> findSameContinentCountryListItem(List<Map<String, String>> continentInfoMapList,
                                                                       List<List<Map<String, String>>> countryInfoMapList,
                                                                       String continentCode) {
        ListIterator<Map<String, String>> iteratorContinentMap = continentInfoMapList.listIterator();
        ListIterator<List<Map<String, String>>> iteratorCountryInfoMap = countryInfoMapList.listIterator();
        while (iteratorContinentMap.hasNext() && iteratorCountryInfoMap.hasNext()) {
            Map<String, String> itemContinentMap = (Map<String, String>) iteratorContinentMap.next();
            List<Map<String, String>> itemCountryInfoMap = (List<Map<String, String>>) iteratorCountryInfoMap.next();
            if (itemContinentMap.get(MAP_KEY_NAME_CONTINENT_CODE).equals(continentCode)) {
                return itemCountryInfoMap;
            }
        }
        return null;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
