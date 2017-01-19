package com.mediatek.contacts.plugin;

import com.mediatek.contacts.plugin.AlertDialogFragment.EditTextDialogFragment;
import com.mediatek.contacts.plugin.AlertDialogFragment.EditTextDialogFragment.EditTextDoneListener;
import com.mediatek.contacts.plugin.MessageAlertDialogFragment.DoneListener;
import com.mediatek.common.telephony.AlphaTag;

//import com.mediatek.op03.plugin.R;
import com.mediatek.contacts.plugin.AASSNEResources;

import android.R.anim;
import android.app.ActionBar;
import android.app.Activity;
import android.content.Intent;
import android.content.ClipData.Item;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

import java.util.List;

public class AasTagActivity extends Activity {
    private static final String TAG = "CustomAasActivity";

    private static final String CREATE_AAS_TAG_DIALOG = "create_aas_tag_dialog";
    private static final String EDIT_AAS_NAME = "edit_aas_name";
    private static final String DELETE_TAG_DIALOG = "delet_tag_dialog";
    private static final String EDIT_TAG_DIALOG = "edit_tag_dialog";

    private boolean isModifying = false;
    private AasTagInfoAdapter mAasAdapter = null;
    private int mSlotId = -1;
    private AlphaTag mAlphaTag = null;
    private View mActionBarEdit = null;
    private TextView mSelectedView = null;
    private ToastHelper mToastHelper = null;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.custom_aas);
        setContentView(AASSNEResources.custom_aas);

        Intent intent = getIntent();
        if (intent != null) {
            mSlotId = intent.getIntExtra(SimUtils.KEY_SLOT, -1);
        }
        if (mSlotId == -1) {
            LogUtils.e(TAG, "Eorror slotId=-1, finish the CustomAasActivity");
            finish();
        }
        //ListView listView = (ListView) findViewById(AASSNEResources.custom_aas_id); 
	ListView listView = (ListView) findViewById(AASSNEResources.custom_aas_id);
        mAasAdapter = new AasTagInfoAdapter(this, mSlotId);
        mAasAdapter.updateAlphaTags();
        listView.setAdapter(mAasAdapter);
        mToastHelper = new ToastHelper(this);
        listView.setOnItemClickListener(new ListItemClickListener());

        initActionBar();
    }

    public void initActionBar() {
        ActionBar actionBar = getActionBar();
        LayoutInflater inflate = getLayoutInflater();
        //View customView = inflate.inflate(R.layout.custom_aas_action_bar, null);
        View customView = inflate.inflate(AASSNEResources.custom_aas_action_bar, null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM
                | ActionBar.DISPLAY_SHOW_TITLE | ActionBar.DISPLAY_HOME_AS_UP);

        //mActionBarEdit = customView.findViewById(R.id.action_bar_edit);
        mActionBarEdit = customView.findViewById(AASSNEResources.action_bar_edit);
        //mSelectedView = (TextView) customView.findViewById(R.id.selected);
        mSelectedView = (TextView) customView.findViewById(AASSNEResources.selected);
        //ImageView selectedIcon = (ImageView) customView.findViewById(R.id.selected_icon);
        ImageView selectedIcon = (ImageView) customView.findViewById(AASSNEResources.selected_icon);
        selectedIcon.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setMode(AasTagInfoAdapter.MODE_NORMAL);
                updateActionBar();
            }
        });
        actionBar.setCustomView(customView);

        updateActionBar();
    }

    public void updateActionBar() {
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            if (mAasAdapter.isMode(AasTagInfoAdapter.MODE_NORMAL)) {
                actionBar.setDisplayOptions(ActionBar.DISPLAY_HOME_AS_UP,
                        ActionBar.DISPLAY_SHOW_TITLE | ActionBar.DISPLAY_HOME_AS_UP);

                actionBar.setDisplayHomeAsUpEnabled(true);
                actionBar.setDisplayShowTitleEnabled(true);
                //actionBar.setTitle(R.string.aas_custom_title);
                actionBar.setTitle(AASSNEResources.aas_custom_title);
                mActionBarEdit.setVisibility(View.GONE);
            } else {
                actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                        ActionBar.DISPLAY_SHOW_CUSTOM);

                actionBar.setDisplayHomeAsUpEnabled(false);
                actionBar.setDisplayShowTitleEnabled(false);
                mActionBarEdit.setVisibility(View.VISIBLE);
                //String select = getResources().getString(R.string.selected_item_count,
                //        mAasAdapter.getCheckedItemCount());
                String select = getResources().getString(AASSNEResources.selected_item_count,
                        mAasAdapter.getCheckedItemCount());
                mSelectedView.setText(select);
            }
        }
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        LogUtils.d(TAG, "onPrepareOptionsMenu");
        MenuInflater inflater = getMenuInflater();
        menu.clear();
        if (mAasAdapter.isMode(AasTagInfoAdapter.MODE_NORMAL)) {
            //inflater.inflate(R.menu.custom_normal_menu, menu);
            inflater.inflate(AASSNEResources.custom_normal_menu, menu);
        } else {
            //inflater.inflate(R.menu.custom_edit_menu, menu);
	    inflater.inflate(AASSNEResources.custom_edit_menu, menu);
        }

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        super.onOptionsItemSelected(item);
        LogUtils.d(TAG, "onOptionsItemSelected");
        if (mAasAdapter.isMode(AasTagInfoAdapter.MODE_NORMAL)) {
            switch (item.getItemId()) {
            case AASSNEResources.menu_add_new:
                if (!mAasAdapter.isFull()) {
                    showNewAasDialog();
                } else {
                    mToastHelper.showToast(AASSNEResources.aas_usim_full);
                }
                break;
            case AASSNEResources.menu_deletion:
                setMode(AasTagInfoAdapter.MODE_EDIT);
                break;
            case android.R.id.home:
                finish();
                break;
            default:
            }
        } else {
            switch (item.getItemId()) {
            case AASSNEResources.menu_select_all:
                mAasAdapter.setAllChecked(true);
                updateActionBar();
                break;
            case AASSNEResources.menu_disselect_all:
                mAasAdapter.setAllChecked(false);
                updateActionBar();
                break;
            case AASSNEResources.menu_delete:
                // mAasAdapter.deleteCheckedAasTag();
                if (mAasAdapter.getCheckedItemCount() == 0) {
                    mToastHelper.showToast(AASSNEResources.multichoice_no_select_alert);
                } else {
                    showDeleteAlertDialog();
                }
                break;
            default:
                break;
            }
        }
        return true;
    }

    public void setMode(int mode) {
        mAasAdapter.setMode(mode);
        updateActionBar();
        invalidateOptionsMenu();
    }

    @Override
    public void onBackPressed() {
        if (mAasAdapter.isMode(AasTagInfoAdapter.MODE_EDIT)) {
            setMode(AasTagInfoAdapter.MODE_NORMAL);
        } else {
            super.onBackPressed();
        }
    }

    protected void showNewAasDialog() {
        EditTextDialogFragment createItemDialogFragment = EditTextDialogFragment.newInstance(
                AASSNEResources.aas_new_dialog_title, android.R.string.cancel, android.R.string.ok, "");
        createItemDialogFragment.setOnEditTextDoneListener(new NewAlpahTagListener());
        createItemDialogFragment.show(getFragmentManager(), CREATE_AAS_TAG_DIALOG);
    }

    final private class NewAlpahTagListener implements EditTextDoneListener {

        @Override
        public void onClick(String text) {
            if (mAasAdapter.isExist(text)) {
                mToastHelper.showToast(AASSNEResources.aas_name_exist);
            } else if (!SimUtils.isAasTextValid(text, mSlotId)) {
                mToastHelper.showToast(AASSNEResources.aas_name_invalid);
            } else {
                int aasIndex = SimUtils.insertUSIMAAS(mSlotId, text);
                LogUtils.d(TAG, "insertAasTag() aasIndex = " + aasIndex);
                if (aasIndex > 0) {
                    mAasAdapter.updateAlphaTags();
                } else {
                    mToastHelper.showToast(AASSNEResources.aas_new_fail);
                }
            }
        }
    }

    protected void showEditAasDialog(AlphaTag alphaTag) {
        if (alphaTag == null) {
            LogUtils.e(TAG, "showEditAasDialog(): alphaTag is null,");
            return;
        }
        final String text = alphaTag.getAlphaTag();
        EditTextDialogFragment editDialogFragment = EditTextDialogFragment.newInstance(
                AASSNEResources.ass_rename_dialog_title, android.R.string.cancel, android.R.string.ok, text);
        editDialogFragment.setOnEditTextDoneListener(new EditAlpahTagListener(alphaTag));
        editDialogFragment.show(getFragmentManager(), EDIT_AAS_NAME);
    }

    final private class EditAlpahTagListener implements EditTextDoneListener {
        private AlphaTag mAlphaTag;

        public EditAlpahTagListener(AlphaTag alphaTag) {
            mAlphaTag = alphaTag;
        }

        @Override
        public void onClick(String text) {
            if (mAlphaTag.getAlphaTag().equals(text)) {
                LogUtils.d(TAG, "mAlphaTag.getAlphaTag()==text : " + text);
                return;
            }
            if (mAasAdapter.isExist(text)) {
                mToastHelper.showToast(AASSNEResources.aas_name_exist);
            } else if (!SimUtils.isAasTextValid(text, mSlotId)) {
                mToastHelper.showToast(AASSNEResources.aas_name_invalid);
            } else {
                showEditAssertDialog(mAlphaTag, text);
            }
        }
    }

    private void showEditAssertDialog(AlphaTag alphaTag, String targetName) {
        MessageAlertDialogFragment editAssertDialogFragment = MessageAlertDialogFragment
                .newInstance(android.R.string.dialog_alert_title, AASSNEResources.ass_edit_assert_message,
                        true, targetName);
        editAssertDialogFragment.setDeleteDoneListener(new EditAssertListener(alphaTag));
        editAssertDialogFragment.show(getFragmentManager(), EDIT_TAG_DIALOG);
    }

    final private class EditAssertListener implements DoneListener {
        private AlphaTag mAlphaTag = null;

        public EditAssertListener(AlphaTag alphaTag) {
            mAlphaTag = alphaTag;
        }

        @Override
        public void onClick(String text) {
            boolean flag = SimUtils.updateUSIMAAS(mSlotId, mAlphaTag.getRecordIndex(), mAlphaTag
                    .getPbrIndex(), text);
            if (flag) {
                mAasAdapter.updateAlphaTags();
            } else {
                String msg = getResources().getString(AASSNEResources.aas_edit_fail,
                        mAlphaTag.getAlphaTag());
                mToastHelper.showToast(msg);
            }
        }
    }

    protected void showDeleteAlertDialog() {
        MessageAlertDialogFragment deleteDialogFragment = MessageAlertDialogFragment.newInstance(
                android.R.string.dialog_alert_title, AASSNEResources.aas_delele_dialog_message, true, "");
        deleteDialogFragment.setDeleteDoneListener(new DeletionListener());
        deleteDialogFragment.show(getFragmentManager(), DELETE_TAG_DIALOG);
    }

    final private class DeletionListener implements DoneListener {
        @Override
        public void onClick(String text) {
            LogUtils.d(TAG, "DeletionListener");
            mAasAdapter.deleteCheckedAasTag();
            setMode(AasTagInfoAdapter.MODE_NORMAL);
        }
    }

    public class ListItemClickListener implements OnItemClickListener {

        @Override
        public void onItemClick(AdapterView<?> view, View v, int pos, long arg3) {
            if (mAasAdapter.isMode(AasTagInfoAdapter.MODE_NORMAL)) {
                showEditAasDialog(mAasAdapter.getItem(pos).mAlphaTag);
            } else {
                mAasAdapter.updateChecked(pos);
                invalidateOptionsMenu();
                updateActionBar();
            }
        }
    }

}
