package com.mediatek.mediatekdm.wfhs.parser;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

import android.util.Log;

import com.mediatek.mediatekdm.mdm.MdmException;
import com.mediatek.mediatekdm.mdm.MdmTree;

public class IOHandlerInfo {

    private static final String TAG = "DM/Wifi.parser.IOHandlerInfo";
    
    public List<String> getRegisterUris(String hotspot) {
        List<String> uris = new ArrayList<String>();
        if (hasBrother) {
            try {
                String[] xs = mTree.listChildren(String.format("%s/%s", hotspot, localUri));
                for (String x : xs) {
                    Log.i(TAG, "x = " + x);
                    for (String node : preferNodes) {
                        String uri = String.format("%s/%s/%s/%s", hotspot, localUri, x, node);
                        Log.i(TAG, "uri = " + uri);
                        uris.add(uri);
                    }
                }
            } catch (MdmException e) {
                e.printStackTrace();
            }
        } else {
            for (String node : preferNodes) {
                String uri = String.format("%s/%s/%s", hotspot, localUri, node);
                uris.add(uri);
            }
        }        
        return uris;
    }

    public String nodeToKey(String node) {
        for (String localNode : preferNodes) {
            if (node.endsWith(localNode)) {
                return mNodeToKey.get(localNode);
            }
        }
        return null;
    }
    
    public void addPreferSessionId(Integer id) {
        preferSessionIds.add(id);
    }
    
    public void addPreferNode(String node) {
        preferNodes.add(node);
    }
    
    public void addPreferKey(String key) {
        preferKeys.add(key);
    }
    
    /*
     * a node uri whose children leaves need to register node io handler
     */
    public String localUri;
    
    /*
     * node io handler's class name
     */
    public String handlerName;
    
    /*
     * if one node has child node that has same composition
     * such as : Policy/PreferredRoamingPartnerList/x/FQDN_Match
     * PreferredRoamingPartnerList may have several nodes "x"
     */
    public boolean hasBrother;
    
    /*
     * the sessions node io handler should work
     */
    public List<Integer> preferSessionIds = new LinkedList<Integer>();

    /*
     * leaves need to register node io handler
     */
    protected List<String> preferNodes = new LinkedList<String>();

    /*
     * leaves need to register node io handler
     */
    protected List<String> preferKeys = new LinkedList<String>();
    
    /*
     * use the keys to put the leaves value to the intent sending to the DM
     * client
     */
    protected HashMap<String, String> mNodeToKey = new HashMap<String, String>();
    
    protected MdmTree mTree = new MdmTree();
}
