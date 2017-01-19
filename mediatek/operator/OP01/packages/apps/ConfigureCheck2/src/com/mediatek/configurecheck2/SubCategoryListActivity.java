package com.mediatek.configurecheck2;

import com.mediatek.configurecheck2.R;
import com.mediatek.common.featureoption.FeatureOption;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class SubCategoryListActivity extends Activity {

	private static final String TAG = "SubCategoryListActivity";
	private ListView mSubCategoryListView;
	private String[] mSubCategoryArray;
	private ArrayAdapter<String> mSubCategoryAdapter;
	private String mRequiredCategory;
	private Boolean mIsServiceTest;
    private int mTdsCateTitlePos;
    private int mTdlCateTitlePos;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
        CTSCLog.i(TAG, "onCreate()");
		super.onCreate(savedInstanceState);
		setContentView(R.layout.sub_category_list);
		setTitle(getIntent().getStringExtra("Category"));
		
		mSubCategoryListView = (ListView)findViewById(R.id.sub_category_list);
		mIsServiceTest = isServiceTestCategory();
		CTSCLog.i(TAG, "mIsServiceTest = " + mIsServiceTest);
		
		if (mIsServiceTest) {
            mSubCategoryArray = getResources().getStringArray(
                    R.array.service_test_sub_category);
            mSubCategoryAdapter = new ArrayAdapter<String>(this,
                    android.R.layout.simple_list_item_1, mSubCategoryArray);
        } else {
            if (Utils.IS_SGLTE_PHONE) {
                mSubCategoryArray = getResources().getStringArray(
                        R.array.sglte_protocol_sub_category);
                mSubCategoryAdapter = new ProtocolCateAdapter(this, 
                        R.layout.sub_category_item, mSubCategoryArray);
                getCategoryTitlePos(mSubCategoryArray);
            } else if (Utils.IS_CSFB_PHONE){
                mSubCategoryArray = getResources().getStringArray(
                        R.array.csfb_protocol_sub_category);
                mSubCategoryAdapter = new ProtocolCateAdapter(this, 
                        R.layout.sub_category_item, mSubCategoryArray);
                getCategoryTitlePos(mSubCategoryArray);
            } else {
                mSubCategoryArray = getResources().getStringArray(
                        R.array.protocol_conformance_sub_category);
                mSubCategoryAdapter = new ArrayAdapter<String>(this,
                        android.R.layout.simple_list_item_1, mSubCategoryArray);
            }
        }
        mSubCategoryListView.setAdapter(mSubCategoryAdapter);
        
        mSubCategoryListView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                long arg3) {
                // TODO Auto-generated method stub
                CTSCLog.i(TAG, "onItemClick(): category = " + mSubCategoryArray[arg2]);
                if (Utils.IS_SGLTE_PHONE || Utils.IS_CSFB_PHONE) {
                    if (!mIsServiceTest) {
                        if (arg2 == mTdsCateTitlePos || arg2 == mTdlCateTitlePos) {
                            //just do nothing
                            return;
                        }
                    }
                }
                if (false == checkProviderIsEmpty(mSubCategoryArray[arg2])) {
                    Intent intent = new Intent(SubCategoryListActivity.this, CheckResultActivity.class);
                    intent.putExtra("Category", mSubCategoryArray[arg2]);
                    startActivity(intent);
                } else {
                    AlertDialog builder = new AlertDialog.Builder(SubCategoryListActivity.this)
                                                         .setMessage(getString(R.string.str_noitem_message))
                                                         .setPositiveButton(android.R.string.ok, null)
                                                         .create();
                    builder.show();
                }
            }
        });
    }
	
    private void getCategoryTitlePos(String[] categoryArray) {
        for (int i = 0; i < categoryArray.length; i++) {
            if (categoryArray[i].equals(SubCategoryListActivity.this
                    .getString(R.string.TDS_category_title))) {
                mTdsCateTitlePos = i;
            } else if (categoryArray[i].equals(SubCategoryListActivity.this
                    .getString(R.string.TDL_category_title))){
                mTdlCateTitlePos = i;
                break;
            }
        }
    }

    private boolean checkProviderIsEmpty(String catogory) {
        CheckItemProviderBase provider = ProviderFactory.getProvider(this, catogory);
        int count = provider.getItemCount();
        if (0 == count) {
            return true;
        }
        return false;
    }

    public Boolean isServiceTestCategory() {

        Intent intent = getIntent();
        mRequiredCategory = intent.getStringExtra("Category");
        
        if (mRequiredCategory.equals(
        		getString(R.string.protocol_conformance_test))) {
        	return false;
		} else if (mRequiredCategory.equals(
				getString(R.string.service_test))){
			return true;
		} else {
			throw new RuntimeException("No such category!");
		}
	}
	
    private class ProtocolCateAdapter extends ArrayAdapter<String> {
        
        private LayoutInflater mInflater;
        private int mLayoutResId;

        class ViewHolder {
            private TextView mCategoryTitle;
            private TextView mTestType;
        }
        
        private ProtocolCateAdapter(Context context, int resourceId, String[] array) {
            super(context, resourceId, array);
            mInflater = LayoutInflater.from(context);
            mLayoutResId = resourceId;
        }
        
        public View getView(int position, View convertView, ViewGroup parent) {
            CTSCLog.d(TAG, "getView() position = " + position + " convertView = " + convertView);
            CTSCLog.d(TAG, "getItem(position) = " + getItem(position));
            ViewHolder holder;
            if (convertView == null) {
                holder = new ViewHolder();
                convertView = mInflater.inflate(mLayoutResId, null);
                
                holder.mCategoryTitle = (TextView)convertView.findViewById(R.id.category_title);   
                holder.mTestType = (TextView)convertView.findViewById(R.id.test_type);
                convertView.setTag(holder);//to associate(store) the holder as a tag in convertView
            } else {
                holder = (ViewHolder)convertView.getTag();
            }
            
            if (position == mTdsCateTitlePos || position == mTdlCateTitlePos) {
                holder.mCategoryTitle.setVisibility(View.VISIBLE);
                holder.mCategoryTitle.setText(getItem(position));
                holder.mTestType.setVisibility(View.GONE);
            } else {
                holder.mCategoryTitle.setVisibility(View.GONE);
                holder.mTestType.setVisibility(View.VISIBLE);
                holder.mTestType.setText(getItem(position));
            }

            return convertView;
        }
    }
}
